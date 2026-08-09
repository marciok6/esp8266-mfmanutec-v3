/**
 * @file esp8266-mfmanutec-v3.ino
 * @brief Firmware principal MFMANUTEC IoT v3
 *
 * Monitoramento remoto de temperatura com sensor DS18B20,
 * acionamento de relé, display OLED, comunicação com API REST,
 * atualização OTA e página de configuração WiFi em modo AP.
 *
 * =============================================================
 *  HARDWARE
 * =============================================================
 *  - ESP8266 NodeMCU / ESP-12F
 *  - Display OLED I2C 0.96" 128x64 SSD1306/SSD1315
 *  - Sensor de temperatura DS18B20
 *  - Módulo Relé
 *  - LED de status
 *  - Botão de reset (GPIO0 / D3)
 *
 * =============================================================
 *  BIBLIOTECAS NECESSÁRIAS (Arduino IDE / PlatformIO)
 * =============================================================
 *  - ESP8266WiFi          (core ESP8266)
 *  - ESP8266WebServer     (core ESP8266)
 *  - ESP8266HTTPClient    (core ESP8266)
 *  - ESP8266httpUpdate    (core ESP8266)
 *  - LittleFS             (core ESP8266)
 *  - ArduinoJson          v6.x  (Benoit Blanchon)
 *  - OneWire              (Paul Stoffregen)
 *  - DallasTemperature    (Miles Burton)
 *  - Adafruit GFX Library (Adafruit)
 *  - Adafruit SSD1306     (Adafruit)
 *
 * =============================================================
 *  CONFIGURAÇÃO DE PINOS – veja Config.h
 * =============================================================
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

// ============================================================
// INCLUDES
// ============================================================
#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "Config.h"
#include "ConfigManager.h"
#include "WiFiController.h"
#include "SensorManager.h"
#include "RelayManager.h"
#include "DisplayManager.h"
#include "ApiManager.h"
#include "OTAManager.h"
#include "WebServerManager.h"

// ============================================================
// INSTÂNCIAS DOS GERENCIADORES
// ============================================================
ConfigManager     configMgr;
WiFiController    wifiCtrl(configMgr);
SensorManager     sensor;
RelayManager      relay;
DisplayManager    display;
ApiManager        api(configMgr);
OTAManager        ota(configMgr);
WebServerManager  webServer(configMgr);

// ============================================================
// VARIÁVEIS GLOBAIS
// ============================================================
unsigned long _startTime         = 0;   // Timestamp do boot (ms)
unsigned long _lastLedBlink      = 0;   // Último toggle do LED
bool          _ledState          = false;
bool          _otaInProgress     = false;
bool          _otaFailureLocked  = false;
unsigned long _otaFailureUntil    = 0;
unsigned long _noConfigStart     = 0;

// ============================================================
// PROTÓTIPOS
// ============================================================
void updateDisplay();
void handleRelay();
void handleOTA();
void blinkLed();
unsigned long uptimeSeconds();

// ============================================================
// SETUP
// ============================================================

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println(F("\n\n========================================"));
    Serial.printf("  MFMANUTEC IoT v%s  –  Inicializando\n", FIRMWARE_VERSION);
    Serial.println(F("========================================\n"));

    _startTime = millis();
    _noConfigStart = 0;

    // LED de status
    pinMode(PIN_LED_STATUS, OUTPUT);
    digitalWrite(PIN_LED_STATUS, LOW);

    // -- Configurações --
    if (!configMgr.begin()) {
        Serial.println(F("[Main] ERRO: LittleFS falhou – operando sem persistência."));
    } else {
        configMgr.load();
    }

    // -- Display --
    if (!display.begin()) {
        Serial.println(F("[Main] Display OLED não inicializado (continuando sem display)."));
    }

    // -- Sensores e relé --
    if (!sensor.begin()) {
        Serial.println(F("[Main] AVISO: sensor DS18B20 não detectado."));
    }
    relay.begin();

    // Restaura estado do relé salvo
    relay.setState(configMgr.config.relayState);

    // -- WiFi --
    wifiCtrl.begin();

    if (wifiCtrl.isApMode()) {
        // Modo AP: inicia servidor de configuração e exibe tela AP
        webServer.begin();
        display.showAPMode(WiFi.softAPSSID(), String(AP_IP_ADDR));
    } else {
        // Modo STA: exibe "Conectando..." enquanto tenta
        display.showConnectingAttempt(configMgr.config.ssid, 1, STA_MAX_ATTEMPTS);
    }

    // -- OTA --
    ota.begin();

    // -- API --
    api.begin();

    Serial.println(F("[Main] Setup concluído.\n"));
}

// ============================================================
// LOOP
// ============================================================

void loop() {
    unsigned long now = millis();

    // -- LED de status --
    blinkLed();

    // -- Botão + gestão WiFi --
    wifiCtrl.handle();

    // -- Servidor web (modo AP) --
    if (wifiCtrl.isApMode()) {
        if (!webServer.isRunning()) {
            Serial.println(F("[Main] Iniciando servidor web do modo AP."));
            webServer.begin();
        }

        webServer.handle();

        if (wifiCtrl.isRecoveryAp()) {
            unsigned long elapsed = millis() - wifiCtrl.getApModeStart();
            unsigned long remaining = (elapsed < AP_RECOVERY_DURATION)
                ? (AP_RECOVERY_DURATION - elapsed) / 1000UL
                : 0UL;

            if (remaining == 0) {
                Serial.println(F("[Main] AP de recuperação expirado – reiniciando ESP."));
                delay(500);
                ESP.restart();
            }

            display.showNoConfig(WiFi.softAPSSID(), remaining);
        }

        // Em modo AP não precisa de sensor / API
        return;
    }

    if (webServer.isRunning()) {
        webServer.stop();
    }

    // -- Sensor de temperatura --
    sensor.handle();

    // -- Display (telas rotativas) --
    updateDisplay();

    if (wifiCtrl.isApMode() && _noConfigStart != 0) {
        unsigned long elapsed = millis() - _noConfigStart;
        unsigned long remaining = (AP_RECOVERY_DURATION > elapsed)
            ? (AP_RECOVERY_DURATION - elapsed) / 1000UL
            : 0UL;

        if (remaining == 0) {
            Serial.println(F("[Main] Timer AP de recuperação esgotado – reiniciando ESP."));
            delay(500);
            ESP.restart();
        }

        display.showNoConfig(WiFi.softAPSSID(), remaining);
        webServer.handle();
        return;
    }

    // -- API (somente quando conectado) --
    if (wifiCtrl.isConnected()) {
        float    temp    = sensor.getTemperature();
        bool     valid   = sensor.isValid();
        bool     relSt   = relay.getState();
        int32_t  rssi    = wifiCtrl.getRSSI();
        float    voltage = (float)ESP.getVcc() / 1000.0f;  // Requer ADC_MODE(ADC_VCC)
        unsigned long up = uptimeSeconds();

        api.handle(temp, valid, relSt, rssi, voltage, up);

        // Processar comando de relé do servidor
        handleRelay();

        // Processar OTA pendente
        handleOTA();
    }
}

// ============================================================
// FUNÇÕES AUXILIARES
// ============================================================

/**
 * @brief Atualiza os dados do display e decide qual tela mostrar.
 */
void updateDisplay() {
    if (_otaInProgress) {
        return;
    }

    if (_otaFailureLocked) {
        if (millis() < _otaFailureUntil) {
            return;
        }
        _otaFailureLocked = false;
    }

    // Atualiza dados para o display
    display.setTemperature(sensor.getTemperature(), sensor.isValid());
    display.setRelayState(relay.getState());
    display.setWiFiInfo(wifiCtrl.getRSSI(), wifiCtrl.getLocalIP());
    display.setClientName(configMgr.config.cliente);
    display.setLocationName(configMgr.config.localInstalacao);
    display.setUptime(uptimeSeconds());
    display.setMinMaxTemp(configMgr.config.minTemperature,
                          configMgr.config.maxTemperature);

    // Exibe tela de erro se sensor falhou
    if (!sensor.isValid() && sensor.getErrorCode() != CODE_NORMAL) {
        display.showError(sensor.getErrorCode(), F("Erro sensor DS18B20"));
        return;
    }

    // Tela de conectando (STA ainda não conectado)
    if (!wifiCtrl.isConnected()) {
        display.showConnectingAttempt(configMgr.config.ssid,
                                     wifiCtrl.getStaAttempts(),
                                     STA_MAX_ATTEMPTS);
        return;
    }

    // Operação normal: display cicla automaticamente
    display.handle();
}

/**
 * @brief Verifica e executa comando de relé recebido da API.
 */
void handleRelay() {
    if (!api.hasRelayCommand()) return;

    bool newState = api.getRelayCommand();
    Serial.printf("[Main] Executando comando de relé: %s\n",
                  newState ? "LIGAR" : "DESLIGAR");

    bool ok = relay.setState(newState);
    if (ok) {
        // Persiste o novo estado
        configMgr.config.relayState = newState;
        configMgr.save();
    }

    // Envia evento para o servidor (código 43 = sucesso, 44 = falha)
    api.sendRelayEvent(ok);
    api.clearFlags();
}

/**
 * @brief Verifica e aplica atualização OTA pendente.
 */
void handleOTA() {
    if (!api.hasPendingOTA()) return;

    _otaInProgress = true;
    _otaFailureLocked = false;
    Serial.println(F("[Main] Iniciando processo OTA..."));
    display.showOTA(0);

    bool ok = ota.applyUpdate([](int pct) {
        display.showOTA(pct);
        yield();  // Evita watchdog durante download
    });

    _otaInProgress = false;

    if (!ok) {
        _otaFailureLocked = true;
        _otaFailureUntil = millis() + 30000UL;
        Serial.printf("[Main] OTA falhou – código: %d\n", ota.getErrorCode());
        display.showError(ota.getErrorCode(), F("Falha na atualizacao"));
        api.clearFlags();
        return;
    }
    // Se ok == true, o dispositivo foi reiniciado dentro de applyUpdate()
}

/**
 * @brief Pisca o LED de status sem usar delay().
 */
void blinkLed() {
    unsigned long now = millis();
    if (now - _lastLedBlink >= INTERVAL_LED_BLINK) {
        _lastLedBlink = now;
        _ledState     = !_ledState;
        digitalWrite(PIN_LED_STATUS, _ledState ? HIGH : LOW);
    }
}

/**
 * @brief Retorna o uptime em segundos desde o boot.
 */
unsigned long uptimeSeconds() {
    return (millis() - _startTime) / 1000UL;
}
