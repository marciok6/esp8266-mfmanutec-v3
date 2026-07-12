/**
 * @file RelayManager.cpp
 * @brief Implementação do gerenciador de relé
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#include "RelayManager.h"

// ============================================================
// CONSTRUTOR
// ============================================================

RelayManager::RelayManager()
    : _state(false), _initialized(false)
{
}

// ============================================================
// INICIALIZAÇÃO
// ============================================================

void RelayManager::begin() {
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);   // Relé desligado na inicialização
    _state       = false;
    _initialized = true;
    Serial.println(F("[Relay] Relé inicializado (desligado)."));
}

// ============================================================
// CONTROLE
// ============================================================

bool RelayManager::setState(bool state) {
    if (!_initialized) {
        Serial.println(F("[Relay] ERRO: relé não inicializado."));
        return false;
    }

    _state = state;
    digitalWrite(PIN_RELAY, state ? HIGH : LOW);

    // Verifica se o pino responde ao comando
    bool pinRead = (digitalRead(PIN_RELAY) == HIGH);
    if (pinRead != state) {
        Serial.println(F("[Relay] AVISO: divergência entre comando e leitura do pino."));
        return false;
    }

    Serial.printf("[Relay] Estado alterado para: %s\n", state ? "LIGADO" : "DESLIGADO");
    return true;
}

bool RelayManager::getState() const {
    return _state;
}

bool RelayManager::toggle() {
    return setState(!_state);
}
