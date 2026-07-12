/**
 * @file WebServerManager.h
 * @brief Servidor web de configuração (modo AP)
 *
 * Serve a página de configuração WiFi com layout Bootstrap responsivo.
 * Oferece endpoints REST para scan de redes, salvar configurações,
 * reiniciar e restaurar padrões.
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "ConfigManager.h"

// ============================================================
// CLASSE WEBSERVERMANAGER
// ============================================================

/**
 * @class WebServerManager
 * @brief Gerencia o servidor HTTP de configuração do modo AP.
 */
class WebServerManager {
public:
    explicit WebServerManager(ConfigManager& cfg);

    /** Inicia o servidor na porta 80. */
    void begin();

    /**
     * Processa requisições HTTP pendentes.
     * Deve ser chamado no loop() principal.
     */
    void handle();

    /** Para o servidor. */
    void stop();

    /** Retorna true se o servidor está ativo. */
    bool isRunning() const;

private:
    ESP8266WebServer _server;
    ConfigManager&   _cfg;
    bool             _running;

    // -- Handlers de rota --
    void _handleRoot();
    void _handleScan();
    void _handleSave();
    void _handleRestart();
    void _handleReset();
    void _handleInfo();
    void _handleNotFound();

    // -- HTML embutido em PROGMEM --
    static const char INDEX_HTML[] PROGMEM;
};

#endif // WEB_SERVER_MANAGER_H
