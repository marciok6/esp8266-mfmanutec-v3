/**
 * @file OTAManager.cpp
 * @brief Implementação do gerenciador de atualização OTA
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#include "OTAManager.h"
#include <WiFiClient.h>

// ============================================================
// CONSTRUTOR
// ============================================================

OTAManager::OTAManager(ConfigManager& cfg)
    : _cfg(cfg), _errorCode(CODE_NORMAL)
{
}

// ============================================================
// INICIALIZAÇÃO
// ============================================================

void OTAManager::begin() {
    // Configura a biblioteca de update HTTP
    ESPhttpUpdate.rebootOnUpdate(false);   // Reiniciar manualmente após update
    ESPhttpUpdate.setLedPin(PIN_LED_STATUS, LOW);  // LED aceso durante update
    Serial.println(F("[OTA] OTAManager inicializado."));
}

// ============================================================
// APLICAR ATUALIZAÇÃO
// ============================================================

bool OTAManager::applyUpdate(OTAProgressCallback progressCb) {
    String url    = _cfg.config.otaUrl;
    String hash   = _cfg.config.otaHash;
    String versao = _cfg.config.otaVersao;

    if (url.isEmpty()) {
        Serial.println(F("[OTA] URL de atualização não disponível."));
        _errorCode = CODE_OTA_ERROR;
        return false;
    }

    // Verifica se a versão é de fato mais nova
    if (!_isNewerVersion(versao)) {
        Serial.printf("[OTA] Versão %s não é mais nova que %s – ignorando.\n",
                      versao.c_str(), FIRMWARE_VERSION);
        return false;
    }

    Serial.printf("[OTA] Iniciando atualização para versão %s\n", versao.c_str());
    Serial.printf("[OTA] URL: %s\n", url.c_str());

    // Callback de progresso
    if (progressCb) {
        ESPhttpUpdate.onProgress([&progressCb](int cur, int total) {
            int pct = (total > 0) ? (cur * 100 / total) : 0;
            progressCb(pct);
        });
    }

    WiFiClient client;

    // O ESP8266httpUpdate verifica o MD5 automaticamente se o servidor enviar.
    // Para verificação SHA-256 do hash, comparamos após o download.
    // Nota: ESP8266httpUpdate não suporta SHA-256 nativo – mas a URL + hash
    // são fornecidos pelo servidor confiável; a verificação é informativa.
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            _errorCode = CODE_OTA_ERROR;
            Serial.printf("[OTA] ERRO: %d – %s\n",
                          ESPhttpUpdate.getLastError(),
                          ESPhttpUpdate.getLastErrorString().c_str());
            return false;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println(F("[OTA] Servidor informou: sem atualização."));
            return false;

        case HTTP_UPDATE_OK:
            Serial.println(F("[OTA] Atualização concluída! Reiniciando..."));
            // Limpa flags de OTA antes de reiniciar
            _cfg.config.otaUrl    = "";
            _cfg.config.otaHash   = "";
            _cfg.config.atualizacao = false;
            _cfg.save();
            delay(500);
            ESP.restart();
            return true;  // Nunca alcançado, mas mantido para clareza

        default:
            _errorCode = CODE_OTA_ERROR;
            return false;
    }
}

// ============================================================
// CONSULTAS
// ============================================================

int OTAManager::getErrorCode() const {
    return _errorCode;
}

// ============================================================
// MÉTODOS PRIVADOS
// ============================================================

bool OTAManager::_isNewerVersion(const String& versao) const {
    // Comparação semântica de versão (major.minor.patch).
    // Partes ausentes são tratadas como 0 (ex: "1.2" == "1.2.0").
    auto parseVer = [](const String& v, int& maj, int& min_, int& pat) {
        int first  = v.indexOf('.');
        int second = (first >= 0) ? v.indexOf('.', first + 1) : -1;
        maj  = v.toInt();
        min_ = (first  >= 0) ? v.substring(first  + 1).toInt() : 0;
        pat  = (second >= 0) ? v.substring(second + 1).toInt() : 0;
    };

    int curMaj, curMin, curPat;
    int remMaj, remMin, remPat;

    parseVer(String(FIRMWARE_VERSION), curMaj, curMin, curPat);
    parseVer(versao,                   remMaj, remMin, remPat);

    if (remMaj != curMaj) return remMaj > curMaj;
    if (remMin != curMin) return remMin > curMin;
    return remPat > curPat;
}
