/**
 * @file RelayManager.h
 * @brief Gerenciador do relé de acionamento
 *
 * Encapsula o controle do relé com rastreamento de estado
 * e verificação de falhas de comando.
 *
 * @author  MFMANUTEC
 * @version 1.0.0
 */

#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>
#include "Config.h"

// ============================================================
// CLASSE RELAYMANAGER
// ============================================================

/**
 * @class RelayManager
 * @brief Controla o relé conectado ao GPIO definido em Config.h.
 */
class RelayManager {
public:
    RelayManager();

    /** Configura o pino e define estado inicial (desligado). */
    void begin();

    /**
     * Define o estado do relé.
     * @param state  true = ligado, false = desligado.
     * @return       true se o comando foi aplicado com sucesso.
     */
    bool setState(bool state);

    /** Retorna o estado atual do relé (true = ligado). */
    bool getState() const;

    /**
     * Alterna o estado atual do relé.
     * @return  true se a alternância foi bem-sucedida.
     */
    bool toggle();

private:
    bool _state;        ///< Estado atual do relé
    bool _initialized;  ///< Indica se begin() foi chamado
};

#endif // RELAY_MANAGER_H
