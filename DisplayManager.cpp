/**
 * @file DisplayManager.cpp
 * @brief Implementação do gerenciador do display OLED
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#include "DisplayManager.h"

// ============================================================
// CONSTRUTOR
// ============================================================

DisplayManager::DisplayManager()
    : _display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1),
      _temperature(0.0f),
      _tempValid(false),
      _relayState(false),
      _rssi(0),
      _ip(""),
      _clientName(""),
      _locationName(""),
      _uptime(0),
      _minTemp(DEFAULT_MIN_TEMP),
      _maxTemp(DEFAULT_MAX_TEMP),
      _currentScreen(DisplayScreen::STARTUP),
      _lastSwitch(0),
      _initialized(false)
{
}

// ============================================================
// INICIALIZAÇÃO
// ============================================================

bool DisplayManager::begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println(F("[Display] ERRO: display OLED não detectado."));
        return false;
    }

    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    _initialized = true;

    Serial.println(F("[Display] OLED inicializado com sucesso."));
    showStartup();
    return true;
}

// ============================================================
// LOOP NÃO-BLOQUEANTE
// ============================================================

void DisplayManager::handle() {
    if (!_initialized) return;

    // Mantém uma única tela de resumo após a conexão
    if (_currentScreen == DisplayScreen::CONNECTING) {
        _currentScreen = DisplayScreen::TEMPERATURE;
        _lastSwitch = millis();
    }

    _drawTemperatureScreen();
}

// ============================================================
// TELAS ESPECÍFICAS (chamadas explícitas)
// ============================================================

void DisplayManager::showStartup() {
    if (!_initialized) return;
    _display.clearDisplay();

    // Título centralizado
    _display.setTextSize(2);
    _display.setCursor(12, 20);
    _display.print(F("MFMANUTEC"));

    _display.setTextSize(1);
    _display.setCursor(12, 44);
    _display.print(F("Monitoramento IoT"));

    //_display.setCursor(22, 46);
    //_display.print(F("FW v"));
    //_display.print(FIRMWARE_VERSION);

    // Linha decorativa
    _display.drawFastHLine(0, 38, OLED_WIDTH, SSD1306_WHITE);

    //_display.setCursor(0, 56);
    //_display.print(F("Inicializando..."));
    _display.display();
    delay(5000); 
}

void DisplayManager::showAPMode(const String& ssid, const String& ip) {
    if (!_initialized) return;
    _drawHeader("MODO AP");

    _display.setTextSize(1);
    _display.setCursor(0, 18);
    _display.print(F("SSID:"));
    _display.setCursor(0, 27);
    _display.print(ssid.substring(0, 21));  // Limita a 21 chars

    _display.setCursor(0, 39);
    _display.print(F("IP: "));
    _display.print(ip);

    _display.setCursor(0, 51);
    _display.print(F("Senha: 12345678"));

    _display.display();
    _currentScreen = DisplayScreen::AP_MODE;
}

void DisplayManager::showConnecting(const String& ssid) {
    if (!_initialized) return;
    _drawHeader("CONECTANDO");

    _display.setTextSize(1);
    _display.setCursor(0, 22);
    _display.print(F("Rede:"));
    _display.setCursor(0, 32);
    _display.print(ssid.substring(0, 21));

    _display.setCursor(0, 48);
    _display.print(F("Aguarde..."));

    _display.display();
    _currentScreen = DisplayScreen::CONNECTING;
}

void DisplayManager::showConnectingAttempt(const String& ssid, unsigned long attempt, unsigned long maxAttempts) {
    if (!_initialized) return;
    _drawHeader("CONECTANDO");

    _display.setTextSize(1);
    _display.setCursor(0, 18);
    _display.print(F("Rede: "));
    _display.print(ssid.substring(0, 12));

    _display.setCursor(0, 30);
    _display.print(F("Tentativa: "));
    _display.print(attempt);
    _display.print(F("/"));
    _display.print(maxAttempts);

    _display.setCursor(0, 42);
    _display.print(F("Aguarde..."));

    _display.display();
    _currentScreen = DisplayScreen::CONNECTING;
}

void DisplayManager::showNoConfig(const String& ssid, unsigned long remainingSeconds) {
    if (!_initialized) return;
    _drawHeader("SEM CONFIG");

    _display.setTextSize(1);
    _display.setCursor(0, 18);
    _display.print(F("AP: "));
    _display.print(ssid.substring(0, 15));

    _display.setCursor(0, 30);
    _display.print(F("Reiniciando em: "));
    _display.print(remainingSeconds / 60UL);
    _display.print(F("m"));

    _display.setCursor(0, 42);
    _display.print(F("Acesse 192.168.4.1"));

    _display.display();
    _currentScreen = DisplayScreen::AP_MODE;
}

void DisplayManager::showError(int code, const String& msg) {
    if (!_initialized) return;
    _drawHeader("ERRO");

    _display.setTextSize(1);
    _display.setCursor(0, 18);
    _display.print(F("Codigo: "));
    _display.print(code);

    // Quebra a mensagem em até 2 linhas de 21 chars
    _display.setCursor(0, 30);
    _display.print(msg.substring(0, 21));
    if (msg.length() > 21) {
        _display.setCursor(0, 40);
        _display.print(msg.substring(21, 42));
    }

    _display.setCursor(0, 54);
    _display.print(F("Tentando novamente..."));

    _display.display();
    _currentScreen = DisplayScreen::ERROR;
}

void DisplayManager::showOTA(int progress) {
    if (!_initialized) return;
    _drawHeader("ATUALIZANDO...");

    _display.setTextSize(1);
    _display.setCursor(20, 24);
    _display.print(progress);
    _display.print(F("%"));

    // Barra de progresso
    int barW = (OLED_WIDTH - 4) * progress / 100;
    _display.drawRect(2, 38, OLED_WIDTH - 4, 12, SSD1306_WHITE);
    _display.fillRect(2, 38, barW, 12, SSD1306_WHITE);

    _display.setCursor(0, 54);
    _display.print(F("Nao desligue!"));

    _display.display();
}

// ============================================================
// SETTERS DE DADOS OPERACIONAIS
// ============================================================

void DisplayManager::setTemperature(float temp, bool valid) {
    _temperature = temp;
    _tempValid   = valid;
}

void DisplayManager::setRelayState(bool state) {
    _relayState = state;
}

void DisplayManager::setWiFiInfo(int32_t rssi, const String& ip) {
    _rssi = rssi;
    _ip   = ip;
}

void DisplayManager::setClientName(const String& clientName) {
    _clientName = clientName;
}

void DisplayManager::setLocationName(const String& locationName) {
    _locationName = locationName;
}

void DisplayManager::setUptime(unsigned long seconds) {
    _uptime = seconds;
}

void DisplayManager::setMinMaxTemp(float minT, float maxT) {
    _minTemp = minT;
    _maxTemp = maxT;
}

void DisplayManager::setScreen(DisplayScreen screen) {
    _currentScreen = screen;
    _lastSwitch    = millis();
}

// ============================================================
// MÉTODOS PRIVADOS DE DESENHO
// ============================================================

void DisplayManager::_drawHeader(const char* title) {
    _display.clearDisplay();
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.print(title);
    _display.drawFastHLine(0, 10, OLED_WIDTH, SSD1306_WHITE);
}

void DisplayManager::_drawTemperatureScreen() {
    _display.clearDisplay();

    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.print(_clientName.length() > 0 ? _clientName.substring(0, 12) : F("Local"));

    _drawSignalBars(OLED_WIDTH - 20, 1);
    _display.drawFastHLine(0, 10, OLED_WIDTH, SSD1306_WHITE);

    _display.setTextSize(1);
    _display.setCursor(0, 14);
    _display.print(_locationName.length() > 0 ? _locationName.substring(0, 12) : F("Local"));


    //_display.drawRect(2, 20, 66, 18, SSD1306_WHITE);
    _display.setTextSize(2);
    _display.setCursor(0, 27);
    if (_tempValid) {
        char buf[8];
        dtostrf(_temperature, 5, 1, buf);
        _display.print(buf);
        //_display.write(0xF8);
        //_display.print(F("C"));
    } else {
        _display.print(F("ERRO"));
    }

    _display.setTextSize(1);
    _display.setCursor(87, 14);
    _display.print(F("Saida"));

    _display.drawRect(80, 25, 40, 16, SSD1306_WHITE);
    _display.setTextSize(1);
    _display.setCursor(92, 30);
    _display.print(_relayState ? F("ON") : F("OFF"));

     _display.drawFastHLine(0, 53, OLED_WIDTH, SSD1306_WHITE);

    _display.setCursor(0, 57);
    _display.print(F("ID:"));
    char chipId[12];
    snprintf(chipId, sizeof(chipId), "%06X", static_cast<unsigned long>(ESP.getChipId()));
    _display.print(chipId);

    _display.setCursor(95, 57);    
    _display.print(F("V:"));
    _display.print(FIRMWARE_VERSION);

    _display.display();
}

void DisplayManager::_drawRelayScreen() {
    _drawHeader("STATUS RELE");

    _display.setTextSize(2);
    _display.setCursor(14, 18);
    _display.print(_relayState ? F("LIGADO") : F(" DESL."));

    _display.setTextSize(1);
    _display.setCursor(0, 42);
    _display.print(F("Uptime: "));

    unsigned long h = _uptime / 3600;
    unsigned long m = (_uptime % 3600) / 60;
    unsigned long s = _uptime % 60;
    char upBuf[12];
    snprintf(upBuf, sizeof(upBuf), "%02lu:%02lu:%02lu", h, m, s);
    _display.print(upBuf);

    _display.setCursor(0, 54);
    _display.print(F("FW: v"));
    _display.print(FIRMWARE_VERSION);

    _display.display();
}

void DisplayManager::_drawWiFiScreen() {
    _drawHeader("WiFi");

    _display.setTextSize(1);
    _display.setCursor(0, 14);
    _display.print(F("IP: "));
    _display.print(_ip.substring(0, 17));

    _display.setCursor(0, 26);
    _display.print(F("RSSI: "));
    _display.print(_rssi);
    _display.print(F(" dBm"));

    // Ícone de sinal (barras simples)
    int bars = 0;
    if      (_rssi >= -55) bars = 4;
    else if (_rssi >= -65) bars = 3;
    else if (_rssi >= -75) bars = 2;
    else if (_rssi >= -85) bars = 1;

    for (int i = 0; i < 4; i++) {
        int barH = 4 + i * 3;
        int barX = 96 + i * 7;
        int barY = 26 - barH + 8;
        if (i < bars) {
            _display.fillRect(barX, barY, 5, barH, SSD1306_WHITE);
        } else {
            _display.drawRect(barX, barY, 5, barH, SSD1306_WHITE);
        }
    }

    _display.setCursor(0, 40);
    _display.print(F("Temp: "));
    if (_tempValid) {
        _display.print(_temperature, 1);
        _display.write(0xF8);
        _display.print(F("C"));
    } else {
        _display.print(F("ERRO"));
    }

    _display.display();
}

void DisplayManager::_drawClientScreen() {
    _drawHeader("CLIENTE");

    _display.setTextSize(1);
    _display.setCursor(0, 16);
    _display.print(F("Cliente:"));

    _display.setCursor(0, 28);
    String clientName = _clientName.length() > 0 ? _clientName : F("Sem cliente");
    _display.print(clientName.substring(0, 21));

    _display.setCursor(0, 44);
    _display.print(F("Status: "));
    _display.print(_clientName.length() > 0 ? F("Cadastrado") : F("Pendente"));

    _display.display();
}

void DisplayManager::_drawSignalBars(int16_t x, int16_t y) {
    int bars = 0;
    if (_rssi >= -55) bars = 4;
    else if (_rssi >= -65) bars = 3;
    else if (_rssi >= -75) bars = 2;
    else if (_rssi >= -85) bars = 1;

    for (int i = 0; i < 4; i++) {
        int barH = 3 + i * 2;
        int barX = x + i * 4;
        int barY = y + 6 - barH;
        if (i < bars) {
            _display.fillRect(barX, barY, 3, barH, SSD1306_WHITE);
        } else {
            _display.drawRect(barX, barY, 3, barH, SSD1306_WHITE);
        }
    }
}
