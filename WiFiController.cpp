/**
 * @file WiFiController.cpp
 * @brief Implementação do gerenciador de conectividade WiFi
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#include "WiFiController.h"

// ============================================================
// CONSTRUTOR
// ============================================================

WiFiController::WiFiController(ConfigManager& cfg)
    : _cfg(cfg),
      _state(WiFiControllerState::IDLE),
      _lastCheck(0),
      _connectStart(0),
      _btnPressStart(0),
      _btnPressed(false)
{
}

// ============================================================
// INICIALIZAÇÃO
// ============================================================

void WiFiController::begin() {
    // Configura o pino do botão com pull-up interno
    pinMode(PIN_RESET_BTN, INPUT_PULLUP);

    WiFi.persistent(false);  // Evita gravação automática no flash
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_OFF);

    // Decide o modo inicial
    if (!_cfg.hasWifiConfig()) {
        Serial.println(F("[WiFi] Sem credenciais – iniciando modo AP."));
        startAP();
    } else {
        startSTA();
    }
}

// ============================================================
// LOOP NÃO-BLOQUEANTE
// ============================================================

void WiFiController::handle() {
    unsigned long now = millis();

    // Verifica o botão de reset em qualquer estado
    if (_checkResetButton()) {
        Serial.println(F("[WiFi] Reset detectado – entrando em modo AP."));
        _cfg.reset();
        _cfg.save();
        startAP();
        return;
    }

    switch (_state) {

        case WiFiControllerState::CONNECTING: {
            // Verifica se conectou
            if (WiFi.status() == WL_CONNECTED) {
                _state = WiFiControllerState::CONNECTED;
                Serial.printf("[WiFi] Conectado! IP: %s | RSSI: %d dBm\n",
                              WiFi.localIP().toString().c_str(),
                              WiFi.RSSI());
                return;
            }
            // Verifica timeout
            if (now - _connectStart >= WIFI_CONNECT_TIMEOUT) {
                Serial.println(F("[WiFi] Timeout de conexão – iniciando modo AP."));
                startAP();
            }
            break;
        }

        case WiFiControllerState::CONNECTED: {
            // Monitora desconexão
            if (WiFi.status() != WL_CONNECTED) {
                _state = WiFiControllerState::DISCONNECTED;
                _lastCheck = now;
                Serial.println(F("[WiFi] Conexão perdida."));
            }
            break;
        }

        case WiFiControllerState::DISCONNECTED: {
            // Tenta reconectar periodicamente
            if (now - _lastCheck >= INTERVAL_RECONNECT) {
                _lastCheck = now;
                Serial.println(F("[WiFi] Tentando reconectar..."));
                startSTA();
            }
            break;
        }

        default:
            break;
    }
}

// ============================================================
// MODO ACCESS POINT
// ============================================================

void WiFiController::startAP() {
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_AP);

    String ssid = _buildApSSID();

    // Configura IP fixo 192.168.4.1
    IPAddress apIP(192, 168, 4, 1);
    IPAddress apGW(192, 168, 4, 1);
    IPAddress apMask(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, apGW, apMask);
    WiFi.softAP(ssid.c_str(), AP_PASSWORD);

    _state = WiFiControllerState::AP_MODE;

    Serial.printf("[WiFi] Modo AP iniciado – SSID: %s | IP: %s\n",
                  ssid.c_str(), AP_IP_ADDR);
}

// ============================================================
// MODO STATION
// ============================================================

void WiFiController::startSTA() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(false);
    WiFi.begin(_cfg.config.ssid.c_str(), _cfg.config.password.c_str());

    _state = WiFiControllerState::CONNECTING;
    _connectStart = millis();

    Serial.printf("[WiFi] Conectando à rede: %s\n", _cfg.config.ssid.c_str());
}

// ============================================================
// CONSULTAS DE ESTADO
// ============================================================

bool WiFiController::isConnected() const {
    return _state == WiFiControllerState::CONNECTED;
}

bool WiFiController::isApMode() const {
    return _state == WiFiControllerState::AP_MODE;
}

WiFiControllerState WiFiController::getState() const {
    return _state;
}

String WiFiController::getSSID() const {
    return WiFi.SSID();
}

String WiFiController::getMacAddress() const {
    return WiFi.macAddress();
}

String WiFiController::getLocalIP() const {
    if (_state == WiFiControllerState::AP_MODE) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

int32_t WiFiController::getRSSI() const {
    return WiFi.RSSI();
}

// ============================================================
// MÉTODOS PRIVADOS
// ============================================================

bool WiFiController::_checkResetButton() {
    bool btnDown = (digitalRead(PIN_RESET_BTN) == LOW);  // Ativo-baixo com pull-up

    if (btnDown) {
        if (!_btnPressed) {
            _btnPressed    = true;
            _btnPressStart = millis();
        } else {
            // A subtração de unsigned long é overflow-safe em C++ (wraps correctamente
            // após ~49 dias sem reinicialização).
            if ((unsigned long)(millis() - _btnPressStart) >= RESET_HOLD_TIME) {
                _btnPressed = false;  // Evita disparar novamente sem soltar o botão
                return true;
            }
        }
    } else {
        _btnPressed = false;
    }
    return false;
}

String WiFiController::_buildApSSID() const {
    // Obtém os últimos 8 dígitos hex do Chip ID (32 bits)
    uint32_t chipId = ESP.getChipId();
    char suffix[9];
    snprintf(suffix, sizeof(suffix), "%08X", chipId);
    return String(AP_SSID_PREFIX) + String(suffix);
}
