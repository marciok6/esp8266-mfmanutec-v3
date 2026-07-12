/**
 * @file ApiManager.cpp
 * @brief Implementação do gerenciador de API REST MFMANUTEC
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#include "ApiManager.h"

// BearSSL SHA-256 (incluso no core do ESP8266 Arduino)
#include <bearssl/bearssl_hash.h>
#include <math.h>

// ============================================================
// CONSTRUTOR
// ============================================================

ApiManager::ApiManager(ConfigManager& cfg)
    : _cfg(cfg),
      _lastSync(0),
      _lastData(0),
      _lastDeviceCode(CODE_NORMAL),
      _lastServerCode(0),
      _pendingOTA(false),
      _relayCommand(false),
      _relayCommandState(false)
{
}

// ============================================================
// INICIALIZAÇÃO
// ============================================================

void ApiManager::begin() {
    // Escalone os primeiros envios para não ocorrer ao mesmo tempo
    _lastSync = millis();
    _lastData = millis();
    Serial.println(F("[API] ApiManager inicializado."));
}

// ============================================================
// LOOP NÃO-BLOQUEANTE
// ============================================================

void ApiManager::handle(float temperature,
                        bool  tempValid,
                        bool  relayState,
                        int32_t rssi,
                        float voltage,
                        unsigned long uptimeSec) {
    unsigned long now = millis();

    // -- Modo 1: SYNC a cada 30 s --
    if (now - _lastSync >= INTERVAL_SYNC) {
        _lastSync = now;
        Serial.println(F("[API] Enviando SYNC (modo 1)..."));
        _sendSync(temperature, tempValid, relayState, rssi, voltage, uptimeSec);
    }

    // -- Modo 2: DATA a cada expected_interval --
    unsigned long dataInterval = (unsigned long)_cfg.config.expectedInterval * 1000UL;
    if (dataInterval == 0) dataInterval = (unsigned long)DEFAULT_INTERVAL * 1000UL;

    if (now - _lastData >= dataInterval) {
        _lastData = now;
        Serial.println(F("[API] Enviando DATA (modo 2)..."));
        _sendData(temperature, tempValid, relayState, rssi, voltage, uptimeSec);
    }
}

// ============================================================
// ENVIO DE EVENTO DE RELÉ (código 43 / 44)
// ============================================================

void ApiManager::sendRelayEvent(bool success) {
    int code = success ? CODE_CMD_SUCCESS : CODE_CMD_FAIL;
    Serial.printf("[API] Enviando evento de relé – código %d\n", code);
    _sendEvent(code);
}

// ============================================================
// CONSULTAS
// ============================================================

int ApiManager::getLastDeviceCode() const { return _lastDeviceCode; }
int ApiManager::getLastServerCode() const { return _lastServerCode; }
bool ApiManager::hasPendingOTA()    const { return _pendingOTA; }
bool ApiManager::hasRelayCommand()  const { return _relayCommand; }
bool ApiManager::getRelayCommand()  const { return _relayCommandState; }

void ApiManager::clearFlags() {
    _pendingOTA   = false;
    _relayCommand = false;
}

// ============================================================
// ENVIO MODO 1 – SYNC
// ============================================================

bool ApiManager::_sendSync(float temperature, bool tempValid, bool relayState,
                           int32_t rssi, float voltage, unsigned long uptimeSec) {
    int devCode = _pickDeviceCode(tempValid, temperature,
                                  _cfg.config.minTemperature,
                                  _cfg.config.maxTemperature);
    _lastDeviceCode = devCode;

    DynamicJsonDocument doc(512);
    doc["modo"]      = MODE_SYNC;
    doc["chip_id"]   = _chipIdHex();
    doc["assinatura"]= _buildSignature();

    JsonObject data  = doc.createNestedObject("data");
    data["mac"]      = WiFi.macAddress();
    data["firmware"] = FIRMWARE_VERSION;
    data["codigo"]   = devCode;

    String payload;
    serializeJson(doc, payload);

    String response;
    if (!_postJson(payload, response)) {
        Serial.println(F("[API] Falha no envio SYNC."));
        return false;
    }

    _parseSync(response);
    return true;
}

// ============================================================
// ENVIO MODO 2 – DATA (telemetria)
// ============================================================

bool ApiManager::_sendData(float temperature, bool tempValid, bool relayState,
                           int32_t rssi, float voltage, unsigned long uptimeSec) {
    DynamicJsonDocument doc(512);
    doc["modo"]       = MODE_DATA;
    doc["chip_id"]    = _chipIdHex();
    doc["assinatura"] = _buildSignature();

    JsonObject data   = doc.createNestedObject("data");
    data["temperatura"] = tempValid ? temperature : 0.0f;
    data["wifi"]        = rssi;
    data["tensao"]      = voltage;
    data["relay_state"] = relayState ? 1 : 0;
    data["uptime"]      = uptimeSec;
    data["heap"]        = ESP.getFreeHeap();

    String payload;
    serializeJson(doc, payload);

    String response;
    if (!_postJson(payload, response)) {
        Serial.println(F("[API] Falha no envio DATA."));
        return false;
    }

    // Reaproveita o parser completo para aplicar comandos/configurações,
    // salvando em memória apenas quando houver diferença real.
    _parseSync(response);
    Serial.printf("[API] Resposta DATA – código servidor: %d\n", _lastServerCode);

    return true;
}

// ============================================================
// ENVIO DE EVENTO PONTUAL
// ============================================================

bool ApiManager::_sendEvent(int deviceCode) {
    DynamicJsonDocument doc(512);
    doc["modo"]      = MODE_SYNC;
    doc["chip_id"]   = _chipIdHex();
    doc["assinatura"]= _buildSignature();

    JsonObject data  = doc.createNestedObject("data");
    data["mac"]      = WiFi.macAddress();
    data["firmware"] = FIRMWARE_VERSION;
    data["codigo"]   = deviceCode;

    String payload;
    serializeJson(doc, payload);

    String response;
    return _postJson(payload, response);
}

// ============================================================
// HTTP POST
// ============================================================

bool ApiManager::_postJson(const String& payload, String& response) {
    WiFiClient client;
    HTTPClient http;

    if (!http.begin(client, API_SERVER)) {
        Serial.println(F("[API] Falha ao iniciar HTTPClient."));
        return false;
    }

    http.setTimeout(HTTP_TIMEOUT);
    http.addHeader(F("Content-Type"), F("application/json"));
    http.addHeader(F("Accept"),       F("application/json"));

    int httpCode = http.POST(payload);

    if (httpCode == HTTP_CODE_OK) {
        response = http.getString();
        Serial.printf("[API] POST OK – resposta: %s\n", response.c_str());
        http.end();
        return true;
    } else {
        Serial.printf("[API] ERRO HTTP: %d – %s\n",
                      httpCode, http.errorToString(httpCode).c_str());
        http.end();
        return false;
    }
}

// ============================================================
// PARSER DA RESPOSTA SYNC
// ============================================================

void ApiManager::_parseSync(const String& response) {
    DynamicJsonDocument doc(1024);
    if (deserializeJson(doc, response) != DeserializationError::Ok) {
        Serial.println(F("[API] ERRO: JSON de resposta inválido."));
        _lastServerCode = 0;
        return;
    }

    _lastServerCode = doc["codigo"] | 0;
    Serial.printf("[API] Código servidor: %d\n", _lastServerCode);

    bool hasConfigChanges = false;

    // -- config: atualiza parâmetros do dispositivo --
    if (doc.containsKey("config")) {
        DeviceConfig before = _cfg.config;
        JsonObject cfg = doc["config"];

        _cfg.config.status           = cfg["status"]           | _cfg.config.status.c_str();
        _cfg.config.cliente          = cfg["cliente"]          | _cfg.config.cliente.c_str();
        _cfg.config.descricao        = cfg["descricao"]        | _cfg.config.descricao.c_str();
        _cfg.config.localInstalacao  = cfg["local_instalacao"] | _cfg.config.localInstalacao.c_str();
        _cfg.config.semConfiguracao  = cfg["sem_configuracao"] | _cfg.config.semConfiguracao;

        if (cfg.containsKey("expected_interval")) {
            uint32_t ei = cfg["expected_interval"].as<uint32_t>();
            // Aceita apenas valores entre 5 s e 3600 s para evitar misconfigurações
            if (ei >= 5 && ei <= 3600) _cfg.config.expectedInterval = ei;
        }
        if (cfg.containsKey("min_temperature")) {
            _cfg.config.minTemperature = cfg["min_temperature"].as<float>();
        }
        if (cfg.containsKey("max_temperature")) {
            _cfg.config.maxTemperature = cfg["max_temperature"].as<float>();
        }
        if (cfg.containsKey("relay_state")) {
            bool newRelay = (cfg["relay_state"].as<int>() != 0);
            if (newRelay != _cfg.config.relayState) {
                // Sinaliza que há um comando de relé pendente
                _relayCommand      = true;
                _relayCommandState = newRelay;
            }
        }
        // Atualiza o horário com referência local do ESP (uptime HH:MM:SS)
        // e evita usar esse campo como gatilho de persistência.
        unsigned long upSec = millis() / 1000UL;
        unsigned long hh = upSec / 3600UL;
        unsigned long mm = (upSec % 3600UL) / 60UL;
        unsigned long ss = upSec % 60UL;
        char espTime[16];
        snprintf(espTime, sizeof(espTime), "%02lu:%02lu:%02lu", hh, mm, ss);
        _cfg.config.horarioServidor = espTime;
        _cfg.config.atualizacao     = (cfg["atualizacao"].as<int>() != 0);

        hasConfigChanges =
            (_cfg.config.status != before.status) ||
            (_cfg.config.cliente != before.cliente) ||
            (_cfg.config.descricao != before.descricao) ||
            (_cfg.config.localInstalacao != before.localInstalacao) ||
            (_cfg.config.semConfiguracao != before.semConfiguracao) ||
            (_cfg.config.expectedInterval != before.expectedInterval) ||
            (fabsf(_cfg.config.minTemperature - before.minTemperature) > 0.0001f) ||
            (fabsf(_cfg.config.maxTemperature - before.maxTemperature) > 0.0001f) ||
            (_cfg.config.atualizacao != before.atualizacao);

        if (hasConfigChanges) {
            if (_cfg.save()) {
                Serial.println(F("[API] Diferenças detectadas: configurações aplicadas e salvas."));
            } else {
                Serial.println(F("[API] ERRO ao salvar configurações atualizadas."));
            }
        } else {
            Serial.println(F("[API] Configuração recebida sem mudanças; descarte de gravação."));
        }
    }

    // -- command: comando de relé explícito --
    if (doc.containsKey("command")) {
        int cmd = doc["command"]["comando_rele"] | -1;
        if (cmd == 0 || cmd == 1) {
            bool newState = (cmd == 1);
            if (newState != _cfg.config.relayState) {
                _relayCommand      = true;
                _relayCommandState = newState;
                Serial.printf("[API] Comando de relé recebido: %s\n",
                              newState ? "LIGAR" : "DESLIGAR");
            }
        }
    }

    // -- update: OTA --
    if (doc.containsKey("update") && _cfg.config.atualizacao) {
        JsonObject upd = doc["update"];
        String versao  = upd["versao_atualizacao"]  | "";
        String url     = upd["url_atualizacao"]     | "";
        String hash    = upd["hash_atualizacao"]    | "";

        if (url.length() > 0 && hash.length() > 0) {
            bool otaChanged =
                (_cfg.config.otaVersao != versao) ||
                (_cfg.config.otaUrl != url) ||
                (_cfg.config.otaHash != hash);

            if (otaChanged) {
                _cfg.config.otaVersao = versao;
                _cfg.config.otaUrl    = url;
                _cfg.config.otaHash   = hash;

                if (_cfg.save()) {
                    Serial.printf("[API] OTA alterada e salva – versão: %s\n", versao.c_str());
                } else {
                    Serial.println(F("[API] ERRO ao salvar dados de OTA."));
                }
            } else {
                Serial.println(F("[API] OTA recebida sem mudanças; descarte de gravação."));
            }

            _pendingOTA = true;
            Serial.printf("[API] OTA disponível – versão: %s\n", versao.c_str());
        }
    }
}

// ============================================================
// HELPERS
// ============================================================

int ApiManager::_pickDeviceCode(bool tempValid, float temp,
                                float minT, float maxT) const {
    if (!tempValid)             return CODE_SENSOR_READ_ERR;
    if (temp < minT || temp > maxT) return CODE_TEMP_OUT_RANGE;
    return CODE_NORMAL;
}

String ApiManager::_chipIdHex() const {
    char buf[9];
    snprintf(buf, sizeof(buf), "%08X", ESP.getChipId());
    return String(buf);
}

String ApiManager::_buildSignature() const {
    // SHA256(CHIP_ID + SECRET_KEY)
    String input = _chipIdHex() + String(API_SECRET_KEY);
    return sha256Hex(input);
}

// ============================================================
// SHA-256 USANDO BEARSSL (incluso no core ESP8266 Arduino)
// ============================================================

String ApiManager::sha256Hex(const String& input) {
    br_sha256_context ctx;
    uint8_t hash[32];

    br_sha256_init(&ctx);
    br_sha256_update(&ctx, input.c_str(), input.length());
    br_sha256_out(&ctx, hash);

    // Converte para string hexadecimal minúscula
    char hexStr[65];
    for (int i = 0; i < 32; i++) {
        snprintf(&hexStr[i * 2], 3, "%02x", hash[i]);
    }
    hexStr[64] = '\0';
    return String(hexStr);
}
