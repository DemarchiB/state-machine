/* ------------------------------- test_fsm.c ------------------------------- */
/**
 * @file test_fsm.c
 * @brief Teste em host da fsm, sobre o exemplo da catraca: estado a cada
 *        passo, evento ignorado sem transicao, e trace (SM_TRACE = 1 so
 *        neste alvo) contendo exatamente as transicoes reais, em ordem.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include "sm/fsm.h"

#include <stdio.h>

#include "catraca.h"
#include "check.h"

#define TRACE_MAX 16

static fsm_state_fn trace_de[TRACE_MAX];
static fsm_state_fn trace_para[TRACE_MAX];
static size_t       trace_len;

void fsm_trace(fsm_state_fn source, fsm_state_fn target)
{
    CHECK(trace_len < TRACE_MAX);
    trace_de[trace_len]   = source;
    trace_para[trace_len] = target;
    trace_len++;
}

static void envia(fsm_t *const me, uint16_t sig)
{
    sm_event_t ev;

    ev.sig = sig;
    fsm_dispatch(me, &ev);
}

int main(void)
{
    fsm_t catraca;

    fsm_init(&catraca, st_travada);
    CHECK(fsm_is_in(&catraca, st_travada));
    CHECK(trace_len == 0U); /* fsm_init nao e transicao */

    envia(&catraca, EV_MOEDA);
    CHECK(fsm_is_in(&catraca, st_liberada));

    envia(&catraca, EV_MOEDA); /* moeda perdida: ignorado */
    CHECK(fsm_is_in(&catraca, st_liberada));

    envia(&catraca, EV_EMPURRAR);
    CHECK(fsm_is_in(&catraca, st_travada));

    envia(&catraca, EV_EMPURRAR); /* ja travada: ignorado */
    CHECK(fsm_is_in(&catraca, st_travada));

    envia(&catraca, EV_FALHA_SENSOR);
    CHECK(fsm_is_in(&catraca, st_manutencao));

    envia(&catraca, EV_MOEDA); /* em manutencao: ignorado */
    CHECK(fsm_is_in(&catraca, st_manutencao));

    envia(&catraca, EV_RESET);
    CHECK(fsm_is_in(&catraca, st_travada));

    /* So as transicoes reais entraram no trace -- nenhum evento ignorado. */
    CHECK(trace_len == 4U);
    CHECK(trace_de[0] == st_travada    && trace_para[0] == st_liberada);
    CHECK(trace_de[1] == st_liberada   && trace_para[1] == st_travada);
    CHECK(trace_de[2] == st_travada    && trace_para[2] == st_manutencao);
    CHECK(trace_de[3] == st_manutencao && trace_para[3] == st_travada);

    (void)printf("test_fsm: OK\n");
    return 0;
}
