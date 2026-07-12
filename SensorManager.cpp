/**
 * @file SensorManager.cpp
 * @brief Implementação do gerenciador do sensor DS18B20
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#include "SensorManager.h"

// ============================================================
// CONSTRUTOR
// ============================================================

SensorManager::SensorManager()
    : _oneWire(PIN_DS18B20),
      _sensors(&_oneWire),
      _temperature(0.0f),
      _valid(false),
      _errorCode(CODE_DS18B20_FAIL),
      _lastRequest(0),
      _requesting(false)
{
}

// ============================================================
// INICIALIZAÇÃO
// ============================================================

bool SensorManager::begin() {
    _sensors.begin();

    // Verifica se ao menos um sensor está presente no barramento
    if (_sensors.getDeviceCount() == 0) {
        Serial.println(F("[Sensor] ERRO: nenhum sensor DS18B20 detectado."));
        _errorCode = CODE_DS18B20_FAIL;
        return false;
    }

    // Resolução de 12 bits (0,0625 °C)
    _sensors.setResolution(12);

    // Modo assíncrono: não aguarda a conversão na chamada requestTemperatures()
    _sensors.setWaitForConversion(false);

    Serial.printf("[Sensor] DS18B20 detectado (%u sensor(es)).\n",
                  _sensors.getDeviceCount());

    // Dispara a primeira conversão imediatamente
    _sensors.requestTemperatures();
    _lastRequest = millis();
    _requesting  = true;

    _errorCode = CODE_NORMAL;
    return true;
}

// ============================================================
// LOOP NÃO-BLOQUEANTE
// ============================================================

void SensorManager::handle() {
    unsigned long now = millis();

    if (_requesting) {
        // Aguarda o tempo de conversão antes de ler
        if (now - _lastRequest >= CONVERSION_DELAY_MS) {
            _requesting = false;
            float temp  = _sensors.getTempCByIndex(0);

            // DEVICE_DISCONNECTED_C == -127.0
            if (temp == DEVICE_DISCONNECTED_C) {
                Serial.println(F("[Sensor] ERRO: sensor desconectado."));
                _valid     = false;
                _errorCode = CODE_SENSOR_DISCONN;

            } else if (!_isPlausible(temp)) {
                Serial.printf("[Sensor] AVISO: temperatura implausível: %.2f °C\n", temp);
                _valid     = false;
                _errorCode = CODE_TEMP_INVALID;

            } else {
                _temperature = temp;
                _valid       = true;
                _errorCode   = CODE_NORMAL;
            }

            // Agenda a próxima conversão
            _sensors.requestTemperatures();
            _lastRequest = now;
            _requesting  = true;
        }
    }
}

// ============================================================
// CONSULTAS
// ============================================================

float SensorManager::getTemperature() const {
    return _temperature;
}

bool SensorManager::isValid() const {
    return _valid;
}

int SensorManager::getErrorCode() const {
    return _errorCode;
}

// ============================================================
// MÉTODOS PRIVADOS
// ============================================================

bool SensorManager::_isPlausible(float temp) const {
    // Faixa física plausível para o DS18B20: -55 a +125 °C
    return (temp > -56.0f && temp < 126.0f);
}
