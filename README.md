# MFMANUTEC IoT v3 – Firmware ESP8266

> **Soluções que Movem Conforto e Eficiência**

Firmware profissional para monitoramento remoto de temperatura com acionamento de relé, desenvolvido para ESP8266 (NodeMCU / ESP-12F).

---

## 📦 Funcionalidades

| Recurso | Descrição |
|---|---|
| Sensor DS18B20 | Leitura de temperatura não-bloqueante (resolução 12-bit) |
| Relé | Acionamento remoto via API ou interface web |
| Display OLED | SSD1306/SSD1315 128×64 – telas rotativas de informação |
| Modo AP | Rede WiFi própria para configuração inicial |
| Modo STA | Conexão à rede local com reconexão automática |
| API REST | Sincronização e telemetria com servidor MFMANUTEC |
| OTA | Atualização de firmware por HTTP |
| LittleFS | Persistência de configurações |
| LED de status | Indicação visual por pisca |

---

## 🔌 Pinagem

| Função | GPIO | NodeMCU |
|---|---|---|
| DS18B20 (data) | 14 | D5 |
| Relé | 12 | D6 |
| LED de Status | 13 | D7 |
| Botão Reset | 0 | D3 |
| OLED SDA | 4 | D2 |
| OLED SCL | 5 | D1 |

> Os pinos podem ser alterados em **`Config.h`**, seção `CONFIGURAÇÃO DE PINOS`.

---

## 📡 Modos de Operação WiFi

### Modo AP (Access Point)
Ativado automaticamente quando:
- Não há credenciais WiFi salvas, **ou**
- O botão `D3 (GPIO0)` é mantido pressionado por **10 segundos**.

| Parâmetro | Valor |
|---|---|
| SSID | `MFMANUTEC-XXXXXXXX` |
| Senha | `12345678` |
| IP | `192.168.4.1` |

Acesse `http://192.168.4.1` para configurar a rede WiFi.

### Modo STA (Station)
Conecta à rede informada na página de configuração com reconexão automática em caso de queda.

---

## 🌐 API REST

**Servidor:** `http://www.mfmanutec.com.br/api/iot/receive.php`

### Modo 1 – SYNC (a cada 30 s)
```json
{
  "modo": 1,
  "chip_id": "ABCD1234",
  "assinatura": "SHA256(chip_id + secret_key)",
  "data": {
    "mac": "AA:BB:CC:DD:EE:FF",
    "firmware": "1.0.0",
    "codigo": 0
  }
}
```

### Modo 2 – DATA (a cada `expected_interval` s)
```json
{
  "modo": 2,
  "chip_id": "ABCD1234",
  "assinatura": "...",
  "data": {
    "temperatura": 45.5,
    "wifi": -65,
    "tensao": 3.3,
    "relay_state": 0,
    "uptime": 3600
  }
}
```

---

## 🔧 Instalação

### Arduino IDE

1. Instale o **ESP8266 Board Package** via Gerenciador de Placas.
2. Selecione **NodeMCU 1.0 (ESP-12E Module)**.
3. Instale as bibliotecas:
   - `ArduinoJson` (Benoit Blanchon) v6.x
   - `OneWire` (Paul Stoffregen)
   - `DallasTemperature` (Miles Burton)
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
4. Abra `esp8266-mfmanutec-v3.ino`.
5. Compile e faça o upload.

> **ADC:** Para leitura de tensão interna (`ESP.getVcc()`), adicione ao início do sketch:
> ```cpp
> ADC_MODE(ADC_VCC);
> ```
> Isso já está presente em `esp8266-mfmanutec-v3.ino`.

### PlatformIO

```bash
# Compilar
pio run

# Upload
pio run --target upload

# Monitor serial
pio device monitor
```

---

## 📁 Estrutura de Arquivos

```
esp8266-mfmanutec-v3/
├── esp8266-mfmanutec-v3.ino  # Sketch principal
├── Config.h                   # Pinos, constantes e códigos
├── ConfigManager.h/.cpp       # Persistência em LittleFS
├── WiFiController.h/.cpp      # Gestão WiFi AP/STA
├── SensorManager.h/.cpp       # Sensor DS18B20
├── RelayManager.h/.cpp        # Controle do relé
├── DisplayManager.h/.cpp      # Display OLED
├── ApiManager.h/.cpp          # API REST
├── OTAManager.h/.cpp          # Atualização OTA
├── WebServerManager.h/.cpp    # Servidor de configuração
├── platformio.ini             # Configuração PlatformIO
└── README.md
```

---

## 🛡️ Códigos de Status

O firmware usa um mapa de códigos documentado em `Config.h` para comunicar o estado do dispositivo ao servidor (0–44) e interpretar respostas do servidor (100–107).

---

## 📄 Licença

© 2026 MFMANUTEC – Todos os direitos reservados.
