/**
 * @file WiFiController.h
 * @brief Gerenciador de conectividade WiFi (modos AP e STA)
 *
 * Controla a transição entre modo Access Point (configuração)
 * e modo Station (operação normal). Inclui detecção do botão
 * de reset para forçar o modo AP.
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Config.h"
#include "ConfigManager.h"

// ============================================================
// ESTADOS DO GERENCIADOR WIFI
// ============================================================
enum class WiFiState {
    IDLE,           ///< Estado inicial / indefinido
    AP_MODE,        ///< Modo Access Point ativo
    CONNECTING,     ///< Tentando conectar à rede STA
    CONNECTED,      ///< Conectado à rede STA
    DISCONNECTED    ///< Desconectado – tentando reconectar
};

// ============================================================
// CLASSE WIFICONTROLLER
// ============================================================

/**
 * @class WiFiController
 * @brief Gerencia os modos AP e STA do ESP8266.
 *
 * Detecta pressão do botão RESET por 10 s e força o modo AP.
 * Em modo STA, realiza reconexão automática sem bloquear o loop.
 */
class WiFiController {
public:
    explicit WiFiController(ConfigManager& cfg);

    /** Inicializa o controlador – deve ser chamado no setup(). */
    void begin();

    /**
     * Processa lógica não-bloqueante de WiFi.
     * Deve ser chamado no loop() principal.
     */
    void handle();

    // -- Consultas de estado --
    bool       isConnected()   const;
    bool       isApMode()      const;
    WiFiState  getState()      const;
    String     getSSID()       const;
    String     getMacAddress() const;
    String     getLocalIP()    const;
    int32_t    getRSSI()       const;

    /** Inicia o modo Access Point. */
    void startAP();

    /** Inicia o modo Station com as credenciais salvas. */
    void startSTA();

private:
    ConfigManager& _cfg;        ///< Referência ao gerenciador de configurações
    WiFiState      _state;      ///< Estado atual
    unsigned long  _lastCheck;  ///< Último tick de verificação (ms)
    unsigned long  _connectStart; ///< Timestamp do início da tentativa STA
    unsigned long  _btnPressStart; ///< Timestamp do início de pressão do botão
    bool           _btnPressed; ///< Botão atualmente pressionado

    /** Verifica o botão de reset e retorna true se deve entrar em modo AP. */
    bool _checkResetButton();

    /** Monta o SSID do AP usando os últimos 8 hex do chip ID. */
    String _buildApSSID() const;
};

#endif // WIFI_CONTROLLER_H
