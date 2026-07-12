/**
 * @file SensorManager.h
 * @brief Gerenciador do sensor de temperatura DS18B20
 *
 * Realiza leituras não-bloqueantes usando millis() e
 * categoriza falhas em códigos de erro padronizados.
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Config.h"

// ============================================================
// CLASSE SENSORMANAGER
// ============================================================

/**
 * @class SensorManager
 * @brief Abstrai a leitura do sensor DS18B20.
 *
 * Usa a biblioteca DallasTemperature com conversão assíncrona
 * para não bloquear o loop principal.
 */
class SensorManager {
public:
    SensorManager();

    /** Inicializa o barramento OneWire e detecta o sensor. */
    bool begin();

    /**
     * Processa a leitura não-bloqueante.
     * Deve ser chamado no loop() principal.
     */
    void handle();

    /** Retorna a última temperatura lida em °C. */
    float getTemperature() const;

    /** Retorna true se o sensor foi detectado e leu valor válido. */
    bool  isValid()        const;

    /** Retorna o código de erro da última operação. */
    int   getErrorCode()   const;

private:
    OneWire         _oneWire;     ///< Barramento 1-Wire
    DallasTemperature _sensors;  ///< Biblioteca de alto nível

    float         _temperature;  ///< Última temperatura lida (°C)
    bool          _valid;        ///< Leitura válida?
    int           _errorCode;    ///< Código de erro atual
    unsigned long _lastRequest;  ///< Último pedido de conversão (ms)
    bool          _requesting;   ///< Aguardando conversão?

    static const unsigned long CONVERSION_DELAY_MS = 800UL; ///< Tempo de conversão 12-bit

    /** Valida se a temperatura está dentro de um intervalo razoável. */
    bool _isPlausible(float temp) const;
};

#endif // SENSOR_MANAGER_H
