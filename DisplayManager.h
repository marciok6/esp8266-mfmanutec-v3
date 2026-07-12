/**
 * @file DisplayManager.h
 * @brief Gerenciador do display OLED SSD1306/SSD1315 (128×64, I2C)
 *
 * Oferece múltiplas telas (startup, AP, conectando, operação,
 * erro, OTA) com alternância automática por millis().
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"

// ============================================================
// TELAS DISPONÍVEIS
// ============================================================
enum class DisplayScreen {
    STARTUP,      ///< Tela de boot com logo MFMANUTEC
    AP_MODE,      ///< Modo AP – exibe SSID e IP
    CONNECTING,   ///< Conectando ao WiFi
    TEMPERATURE,  ///< Temperatura atual e faixa
    RELAY_STATUS, ///< Estado do relé e uptime
    WIFI_INFO,    ///< Sinal WiFi e IP
    ERROR         ///< Tela de erro com código
};

// ============================================================
// CLASSE DISPLAYMANAGER
// ============================================================

/**
 * @class DisplayManager
 * @brief Gerencia o display OLED com múltiplas telas.
 */
class DisplayManager {
public:
    DisplayManager();

    /** Inicializa o display via I2C. */
    bool begin();

    /**
     * Atualiza o display no modo de operação normal.
     * Cicla automaticamente entre as telas com base em millis().
     */
    void handle();

    // -- Telas específicas (chamadas explícitas) --
    void showStartup();
    void showAPMode(const String& ssid, const String& ip);
    void showConnecting(const String& ssid);
    void showError(int code, const String& msg);
    void showOTA(int progress);

    // -- Dados para as telas de operação (atualizados pelo loop) --
    void setTemperature(float temp, bool valid);
    void setRelayState(bool state);
    void setWiFiInfo(int32_t rssi, const String& ip);
    void setUptime(unsigned long seconds);
    void setMinMaxTemp(float minT, float maxT);

    /** Define a tela atual de exibição (para ciclo automático). */
    void setScreen(DisplayScreen screen);

private:
    Adafruit_SSD1306 _display;  ///< Instância do driver do display

    // -- Dados de operação --
    float        _temperature;
    bool         _tempValid;
    bool         _relayState;
    int32_t      _rssi;
    String       _ip;
    unsigned long _uptime;
    float        _minTemp;
    float        _maxTemp;

    // -- Controle de tela --
    DisplayScreen  _currentScreen;
    unsigned long  _lastSwitch;   ///< Último ciclo de troca de tela
    bool           _initialized;

    // -- Métodos de desenho internos --
    void _drawTemperatureScreen();
    void _drawRelayScreen();
    void _drawWiFiScreen();
    void _drawHeader(const char* title);
};

#endif // DISPLAY_MANAGER_H
