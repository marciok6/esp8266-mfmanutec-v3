/**
 * @file OTAManager.h
 * @brief Gerenciador de atualização OTA via HTTP
 *
 * Baixa e aplica o firmware a partir da URL fornecida pelo servidor,
 * verifica o hash SHA-256 antes de aplicar e exibe progresso no display.
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include "Config.h"
#include "ConfigManager.h"

// Callback para progresso – permite atualizar o display externamente
using OTAProgressCallback = std::function<void(int percent)>;

// ============================================================
// CLASSE OTAMANAGER
// ============================================================

/**
 * @class OTAManager
 * @brief Gerencia atualizações OTA via download HTTP.
 *
 * O processo é:
 *  1. Receber URL + hash SHA-256 do servidor (via ApiManager).
 *  2. Chamar applyUpdate() para baixar e aplicar o firmware.
 *  3. Se falhar, registrar erro CODE_OTA_ERROR.
 */
class OTAManager {
public:
    explicit OTAManager(ConfigManager& cfg);

    /** Inicializa o gerenciador OTA. */
    void begin();

    /**
     * Aplica a atualização de firmware a partir da URL e hash salvos
     * em ConfigManager.
     *
     * @param progressCb  Callback opcional para exibir progresso (0–100).
     * @return            true em caso de sucesso (reinicia o dispositivo).
     *                    false em caso de falha.
     */
    bool applyUpdate(OTAProgressCallback progressCb = nullptr);

    /** Retorna o código de erro da última tentativa. */
    int getErrorCode() const;

private:
    ConfigManager&   _cfg;
    int              _errorCode;

    // Verifica se a versão da URL difere da versão atual
    bool _isNewerVersion(const String& versao) const;
};

#endif // OTA_MANAGER_H
