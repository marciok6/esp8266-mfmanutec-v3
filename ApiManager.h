/**
 * @file ApiManager.h
 * @brief Gerenciador de comunicação com a API MFMANUTEC
 *
 * Implementa dois modos de comunicação:
 *  - Modo 1 (SYNC): sincronização a cada 30 s.
 *  - Modo 2 (DATA): telemetria de acordo com expected_interval.
 *
 * Calcula a assinatura SHA-256 e trata todos os códigos de
 * resposta do servidor.
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#ifndef API_MANAGER_H
#define API_MANAGER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "ConfigManager.h"

// ============================================================
// CLASSE APIMANAGER
// ============================================================

/**
 * @class ApiManager
 * @brief Gerencia o envio e recepção de dados para a API REST.
 */
class ApiManager {
public:
    explicit ApiManager(ConfigManager& cfg);

    /** Inicializa o gerenciador. */
    void begin();

    /**
     * Processa os timers de envio sem bloquear o loop.
     * @param temperature  Temperatura atual lida pelo SensorManager.
     * @param tempValid    True se a leitura é válida.
     * @param relayState   Estado atual do relé.
     * @param rssi         RSSI WiFi (dBm).
     * @param voltage      Tensão de alimentação (V) – ADC interno.
     * @param uptimeSec    Tempo ligado em segundos.
     */
    void handle(float temperature,
                bool  tempValid,
                bool  relayState,
                int32_t rssi,
                float voltage,
                unsigned long uptimeSec);

    /**
     * Força o envio imediato de um evento de estado do relé (código 43/44).
     * @param success  true = código 43 (sucesso), false = código 44 (falha).
     */
    void sendRelayEvent(bool success);

    /** Retorna o último código de dispositivo enviado ao servidor. */
    int getLastDeviceCode() const;

    /** Retorna o último código de resposta recebido do servidor. */
    int getLastServerCode() const;

    /** Retorna true se o servidor retornou OTA pendente. */
    bool hasPendingOTA() const;

    /** Retorna true se o servidor indicou comando de relé. */
    bool hasRelayCommand() const;

    /** Retorna o estado do relé solicitado pelo servidor. */
    bool getRelayCommand() const;

    /** Limpa flags de OTA e comando de relé após tratamento. */
    void clearFlags();

private:
    ConfigManager& _cfg;

    // -- Timers --
    unsigned long _lastSync;     ///< Último envio modo 1
    unsigned long _lastData;     ///< Último envio modo 2

    // -- Estado --
    int  _lastDeviceCode;
    int  _lastServerCode;
    bool _pendingOTA;
    bool _relayCommand;
    bool _relayCommandState;

    // -- Métodos de envio --
    bool _sendSync(float temperature, bool tempValid, bool relayState,
                   int32_t rssi, float voltage, unsigned long uptimeSec);
    bool _sendData(float temperature, bool tempValid, bool relayState,
                   int32_t rssi, float voltage, unsigned long uptimeSec);
    bool _sendEvent(int deviceCode);

    // -- Helpers --
    String _buildSignature() const;
    String _chipIdHex()      const;
    bool   _postJson(const String& payload, String& response);
    void   _parseSync(const String& response);
    int    _pickDeviceCode(bool tempValid, float temp,
                           float minT, float maxT) const;

    // -- SHA-256 --
    static String sha256Hex(const String& input);
};

#endif // API_MANAGER_H
