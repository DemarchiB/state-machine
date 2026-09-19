/* ------------------------------- catraca.h -------------------------------- */
/**
 * @file catraca.h
 * @brief Exemplo de fsm: catraca de controle de acesso com tres estados.
 *        Sinais e funcoes de estado sao publicos para os testes e para o
 *        exemplo de Active Object reaproveitarem a mesma maquina.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#ifndef CATRACA_H
#define CATRACA_H

#include "sm/fsm.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Sinais da aplicacao: comecam em SM_SIG_USER. */
enum {
    EV_MOEDA = SM_SIG_USER, /**< Moeda (ou credito) aceita. */
    EV_EMPURRAR,            /**< Alguem empurrou o braco da catraca. */
    EV_FALHA_SENSOR,        /**< Sensor do mecanismo detectou falha. */
    EV_RESET                /**< Reset manual pelo tecnico. */
};

void st_travada(fsm_t *const me, const sm_event_t *const e);
void st_liberada(fsm_t *const me, const sm_event_t *const e);
void st_manutencao(fsm_t *const me, const sm_event_t *const e);

#ifdef __cplusplus
}
#endif

#endif /* CATRACA_H */
