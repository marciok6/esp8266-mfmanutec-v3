/**
 * @file ConfigManager.cpp
 * @brief Implementação do gerenciador de configurações (LittleFS)
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#include "ConfigManager.h"

// Caminho do arquivo de configuração no LittleFS
const char* ConfigManager::CONFIG_FILE = "/config.json";

// ============================================================
// CONSTRUTOR
// ============================================================

ConfigManager::ConfigManager() : _mounted(false) {
    _setDefaults();
}

// ============================================================
// MÉTODOS PÚBLICOS
// ============================================================

bool ConfigManager::begin() {
    if (!LittleFS.begin()) {
        Serial.println(F("[ConfigManager] ERRO: falha ao montar LittleFS."));
        return false;
    }
    _mounted = true;
    Serial.println(F("[ConfigManager] LittleFS montado com sucesso."));
    return true;
}

bool ConfigManager::load() {
    if (!_mounted) {
        Serial.println(F("[ConfigManager] ERRO: LittleFS não montado."));
        return false;
    }

    if (!LittleFS.exists(CONFIG_FILE)) {
        Serial.println(F("[ConfigManager] Arquivo de configuração não encontrado."));
        return false;
    }

    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println(F("[ConfigManager] ERRO: não foi possível abrir o arquivo."));
        return false;
    }

    // Documento JSON com capacidade suficiente para todos os campos
    DynamicJsonDocument doc(1536);
    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err) {
        Serial.printf("[ConfigManager] ERRO ao parsear JSON: %s\n", err.c_str());
        return false;
    }

    // -- Credenciais WiFi --
    config.ssid              = doc["ssid"]             | "";
    config.password          = doc["password"]         | "";

    // -- Parâmetros do servidor --
    config.status            = doc["status"]           | "";
    config.cliente           = doc["cliente"]          | "";
    config.descricao         = doc["descricao"]        | "";
    config.localInstalacao   = doc["local_instalacao"] | "";
    config.semConfiguracao   = doc["sem_configuracao"] | true;
    config.expectedInterval  = doc["expected_interval"]| (uint32_t)DEFAULT_INTERVAL;
    config.minTemperature    = doc["min_temperature"]  | DEFAULT_MIN_TEMP;
    config.maxTemperature    = doc["max_temperature"]  | DEFAULT_MAX_TEMP;
    config.relayState        = doc["relay_state"]      | false;
    config.horarioServidor   = doc["horario_servidor"] | "";
    config.atualizacao       = doc["atualizacao"]      | false;

    // -- OTA --
    config.otaVersao         = doc["ota_versao"]       | "";
    config.otaUrl            = doc["ota_url"]          | "";
    config.otaHash           = doc["ota_hash"]         | "";

    Serial.println(F("[ConfigManager] Configurações carregadas com sucesso."));
    return true;
}

bool ConfigManager::save() {
    if (!_mounted) {
        Serial.println(F("[ConfigManager] ERRO: LittleFS não montado."));
        return false;
    }

    File file = LittleFS.open(CONFIG_FILE, "w");
    if (!file) {
        Serial.println(F("[ConfigManager] ERRO: não foi possível criar o arquivo."));
        return false;
    }

    DynamicJsonDocument doc(1536);

    // -- Credenciais WiFi --
    doc["ssid"]              = config.ssid;
    doc["password"]          = config.password;

    // -- Parâmetros do servidor --
    doc["status"]            = config.status;
    doc["cliente"]           = config.cliente;
    doc["descricao"]         = config.descricao;
    doc["local_instalacao"]  = config.localInstalacao;
    doc["sem_configuracao"]  = config.semConfiguracao;
    doc["expected_interval"] = config.expectedInterval;
    doc["min_temperature"]   = config.minTemperature;
    doc["max_temperature"]   = config.maxTemperature;
    doc["relay_state"]       = config.relayState;
    doc["horario_servidor"]  = config.horarioServidor;
    doc["atualizacao"]       = config.atualizacao;

    // -- OTA --
    doc["ota_versao"]        = config.otaVersao;
    doc["ota_url"]           = config.otaUrl;
    doc["ota_hash"]          = config.otaHash;

    size_t written = serializeJson(doc, file);
    file.close();

    if (written == 0) {
        Serial.println(F("[ConfigManager] ERRO ao escrever JSON."));
        return false;
    }

    Serial.println(F("[ConfigManager] Configurações salvas com sucesso."));
    return true;
}

bool ConfigManager::reset() {
    if (_mounted && LittleFS.exists(CONFIG_FILE)) {
        LittleFS.remove(CONFIG_FILE);
    }
    _setDefaults();
    Serial.println(F("[ConfigManager] Configurações restauradas para o padrão."));
    return true;
}

bool ConfigManager::hasWifiConfig() const {
    return config.ssid.length() > 0;
}

// ============================================================
// MÉTODOS PRIVADOS
// ============================================================

void ConfigManager::_setDefaults() {
    config.ssid             = "";
    config.password         = "";
    config.status           = "";
    config.cliente          = "";
    config.descricao        = "";
    config.localInstalacao  = "";
    config.semConfiguracao  = true;
    config.expectedInterval = DEFAULT_INTERVAL;
    config.minTemperature   = DEFAULT_MIN_TEMP;
    config.maxTemperature   = DEFAULT_MAX_TEMP;
    config.relayState       = false;
    config.horarioServidor  = "";
    config.atualizacao      = false;
    config.otaVersao        = "";
    config.otaUrl           = "";
    config.otaHash          = "";
}
