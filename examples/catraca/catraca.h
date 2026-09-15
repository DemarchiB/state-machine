/* ------------------------------ catraca.h ---------------------------------- */
/**
 * @file catraca.h
 * @brief Interface do exemplo catraca: sinais de aplicacao e funcoes de
 *        estado, expostos para o teste em host (tests/test_catraca.c)
 *        poder comparar contra catraca.state diretamente.
 */

#ifndef CATRACA_H
#define CATRACA_H

#include "fsm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Sinais da aplicacao comecam em SIG_USER (fsm.h) -- os reservados
 * (SIG_ENTRY/SIG_EXIT) ficam abaixo, pra biblioteca. */
enum {
    EV_MOEDA = SIG_USER,
    EV_EMPURRAR,
    EV_FALHA_SENSOR,
    EV_RESET
};

/* Funcoes de estado da catraca. Nao-static (ao contrario da 1a versao)
 * para que o teste em host consiga comparar catraca.state == st_travada
 * etc. -- nao ha getter, o campo `state` de fsm_t e visivel via fsm.h
 * (privado so por convencao de uso). */
void st_travada(fsm_t *me, const event_t *e);
void st_liberada(fsm_t *me, const event_t *e);
void st_manutencao(fsm_t *me, const event_t *e);

#ifdef __cplusplus
}
#endif

#endif /* CATRACA_H */
