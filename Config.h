/**
 * @file Config.h
 * @brief Configurações centrais do projeto MFMANUTEC IoT v3
 *
 * Todas as definições de pinos, constantes, intervalos e códigos
 * de erro/resposta ficam centralizadas aqui para fácil manutenção.
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// VERSÃO DO FIRMWARE
// ============================================================
#define FIRMWARE_VERSION        "1.0.0"

// ============================================================
//  CONFIGURAÇÃO DE PINOS
// ============================================================
#define PIN_DS18B20     14   // GPIO14 (D5) – Sensor de temperatura DS18B20
#define PIN_RELAY       12   // GPIO12 (D6) – Módulo Relé
#define PIN_LED_STATUS  13   // GPIO13 (D7) – LED de Status
#define PIN_RESET_BTN    0   // GPIO0  (D3) – Botão de Reset (pull-up interno)
// Display OLED I2C – pinos padrão do Arduino/ESP8266
#define PIN_I2C_SDA      4   // GPIO4  (D2) – SDA
#define PIN_I2C_SCL      5   // GPIO5  (D1) – SCL

// ============================================================
// CONFIGURAÇÕES DO DISPLAY OLED
// ============================================================
#define OLED_ADDRESS    0x3C   // Endereço I2C do SSD1306/SSD1315
#define OLED_WIDTH       128   // Largura em pixels
#define OLED_HEIGHT       64   // Altura em pixels

// ============================================================
// CONFIGURAÇÕES DE REDE – MODO AP
// ============================================================
#define AP_SSID_PREFIX  "MFMANUTEC-"
#define AP_PASSWORD     "12345678"
#define AP_IP_ADDR      "192.168.4.1"

// ============================================================
// CONFIGURAÇÕES DO SERVIDOR API
// ============================================================
#define API_SERVER      "http://www.mfmanutec.com.br/api/iot/receive.php"
#define API_SECRET_KEY  "MFMANUTEC@2026#IoT!Secure$Key"

// ============================================================
// TIMERS E INTERVALOS (ms)
// ============================================================
#define INTERVAL_SYNC           30000UL   // Sincronização – modo 1 (30 s)
#define INTERVAL_RECONNECT       5000UL   // Tentativa de reconexão WiFi (5 s)
#define INTERVAL_DISPLAY         4000UL   // Troca de tela do display (4 s)
#define INTERVAL_LED_BLINK        500UL   // Pisca do LED de status (500 ms)
#define RESET_HOLD_TIME         10000UL   // Tempo de pressão do botão reset (10 s)
#define WIFI_CONNECT_TIMEOUT    20000UL   // Timeout de conexão WiFi (20 s)
#define HTTP_TIMEOUT            10000     // Timeout HTTP em ms

// ============================================================
// CONFIGURAÇÕES PADRÃO
// ============================================================
#define DEFAULT_INTERVAL         30       // Intervalo padrão de telemetria (s)
#define DEFAULT_MIN_TEMP          0.0f    // Temperatura mínima padrão (°C)
#define DEFAULT_MAX_TEMP        100.0f    // Temperatura máxima padrão (°C)

// ============================================================
// MODOS DE OPERAÇÃO DA API
// ============================================================
#define MODE_SYNC   1   // Sincronização
#define MODE_DATA   2   // Telemetria

// ============================================================
// CÓDIGOS DE ERRO DO DISPOSITIVO
// ============================================================
#define CODE_NORMAL              0
#define CODE_DS18B20_FAIL        1
#define CODE_TEMP_INVALID        2
#define CODE_SENSOR_READ_ERR     3
#define CODE_TEMP_OUT_RANGE      4
#define CODE_RELAY_FAIL          5
#define CODE_GPIO_INVALID        6
#define CODE_CONFIG_NOT_FOUND    7
#define CODE_SAVE_ERROR          8
#define CODE_READ_ERROR          9
#define CODE_WIFI_DISCONNECTED  10
#define CODE_WIFI_AUTH_FAIL     11
#define CODE_SERVER_UNAVAILABLE 12
#define CODE_HTTP_TIMEOUT       13
#define CODE_SIG_INVALID        14
#define CODE_CHIP_NOT_REG       15
#define CODE_LITTLEFS_ERROR     16
#define CODE_LOW_HEAP           17
#define CODE_WATCHDOG           18
#define CODE_UNEXPECTED_RESET   19
#define CODE_OTA_ERROR          20
#define CODE_FW_INCOMPATIBLE    21
#define CODE_LOW_VOLTAGE        22
#define CODE_HIGH_VOLTAGE       23
#define CODE_INTERNAL_ERROR     24
#define CODE_JSON_INVALID       25
#define CODE_JSON_SEND_ERROR    26
#define CODE_API_RESP_INVALID   27
#define CODE_DNS_NOT_FOUND      28
#define CODE_GW_INACCESSIBLE    29
#define CODE_SYNC_FAIL          30
#define CODE_REMOTE_RESET       31
#define CODE_POWER_RESET        32
#define CODE_AP_MODE_ACTIVE     33
#define CODE_CONFIG_RESTORED    34
#define CODE_SENSOR_DISCONN     35
#define CODE_FLASH_ERROR        36
#define CODE_WIFI_MAX_RETRY     37
#define CODE_INIT_TIMEOUT       38
#define CODE_TEMP_CRITICAL      39
#define CODE_MAINTENANCE        40
#define CODE_CONFIG_SYNCED      41
#define CODE_CONFIG_SYNC_FAIL   42
#define CODE_CMD_SUCCESS        43
#define CODE_CMD_FAIL           44

// ============================================================
// CÓDIGOS DE RESPOSTA DO SERVIDOR
// ============================================================
#define SERVER_OK               100
#define SERVER_INTERNAL_ERR     101
#define SERVER_PAYLOAD_ERR      102
#define SERVER_NOT_REG          103
#define SERVER_SIG_INVALID      104
#define SERVER_REGISTERED       105
#define SERVER_PENDING          106
#define SERVER_RATE_LIMIT       107

#endif // CONFIG_H
