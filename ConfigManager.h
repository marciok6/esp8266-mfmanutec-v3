/**
 * @file ConfigManager.h
 * @brief Gerenciador de configurações usando LittleFS (JSON)
 *
 * Responsável por carregar, salvar e restaurar as configurações
 * do dispositivo de forma persistente.
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "Config.h"

// ============================================================
// ESTRUTURA DE CONFIGURAÇÃO
// ============================================================

/**
 * @struct DeviceConfig
 * @brief Reúne todos os parâmetros configuráveis do dispositivo.
 */
struct DeviceConfig {
    // -- Credenciais WiFi --
    String ssid;
    String password;

    // -- Informações recebidas do servidor (SYNC) --
    String   status;
    String   cliente;
    String   descricao;
    String   localInstalacao;
    bool     semConfiguracao;
    uint32_t expectedInterval;   // em segundos
    float    minTemperature;
    float    maxTemperature;
    bool     relayState;
    String   horarioServidor;
    bool     atualizacao;

    // -- Dados de OTA recebidos do servidor --
    String   otaVersao;
    String   otaUrl;
    String   otaHash;
};

// ============================================================
// CLASSE CONFIGMANAGER
// ============================================================

/**
 * @class ConfigManager
 * @brief Gerencia a persistência das configurações em LittleFS.
 *
 * Uso:
 * @code
 *   ConfigManager cfg;
 *   cfg.begin();
 *   cfg.load();
 *   cfg.config.ssid = "MinhaRede";
 *   cfg.save();
 * @endcode
 */
class ConfigManager {
public:
    ConfigManager();

    /** Inicializa o sistema de arquivos LittleFS. */
    bool begin();

    /** Carrega configurações do arquivo JSON no LittleFS. */
    bool load();

    /** Salva configurações no arquivo JSON do LittleFS. */
    bool save();

    /** Remove o arquivo de configuração e restaura os valores padrão. */
    bool reset();

    /** Retorna true se há credenciais WiFi salvas. */
    bool hasWifiConfig() const;

    /** Configuração ativa do dispositivo. */
    DeviceConfig config;

private:
    static const char* CONFIG_FILE;  ///< Caminho do arquivo JSON no LittleFS
    bool _mounted;                   ///< Indica se o LittleFS foi montado

    /** Define os valores padrão do struct. */
    void _setDefaults();
};

#endif // CONFIG_MANAGER_H
