/* ----------------------------- test_catraca.c ------------------------------ */
/**
 * @file test_catraca.c
 * @brief Teste em host do exemplo catraca: despacha uma sequencia de
 *        eventos e confere, a cada passo, que catraca.state e a funcao de
 *        estado esperada. Sem framework -- assert() interrompe na primeira
 *        falha, e o retorno do main (0/1) e o sinal de pass/fail pro CTest.
 */

#include <assert.h>
#include <stdio.h>

#include "fsm.h"
#include "catraca.h"

/* --- Parte 4: trace de transicoes -------------------------------------------
 * Com FSM_TRACE definido (tests/CMakeLists.txt liga isso so pra este alvo,
 * nao pro exemplo nem pra lib), fsm_tran chama fsm_trace(antigo, novo) a
 * cada transicao REAL. Evento ignorado nunca chama fsm_tran -- logo nunca
 * chama fsm_trace tambem. trace_count no final do teste prova isso: dos 5
 * eventos despachados abaixo, so 4 sao transicoes de verdade (o segundo
 * EV_EMPURRAR, ja em TRAVADA, e ignorado).
 *
 * A implementacao mora aqui (no teste), nao na biblioteca -- fsm.h so
 * declara fsm_trace quando FSM_TRACE esta definido; quem sabe os nomes dos
 * estados pra imprimir algo legivel e quem esta testando.
 */
static int trace_count = 0;

static const char *nome_estado(fsm_state_fn estado)
{
    if (estado == st_travada)    return "TRAVADA";
    if (estado == st_liberada)   return "LIBERADA";
    if (estado == st_manutencao) return "MANUTENCAO";
    return "?";
}

void fsm_trace(fsm_state_fn de, fsm_state_fn para)
{
    printf("trace: %s -> %s\n", nome_estado(de), nome_estado(para));
    trace_count++;
}

int main(void)
{
    fsm_t catraca;

    fsm_init(&catraca, st_travada);
    assert(catraca.state == st_travada); /* fsm_init entra no estado inicial */

    {
        const event_t ev = { .sig = EV_MOEDA };
        fsm_dispatch(&catraca, &ev);
        assert(catraca.state == st_liberada);
    }
    {
        const event_t ev = { .sig = EV_EMPURRAR };
        fsm_dispatch(&catraca, &ev);
        assert(catraca.state == st_travada);
    }
    {
        const event_t ev = { .sig = EV_EMPURRAR };
        fsm_dispatch(&catraca, &ev);
        assert(catraca.state == st_travada);
    }
    {
        const event_t ev = { .sig = EV_FALHA_SENSOR };
        fsm_dispatch(&catraca, &ev);
        assert(catraca.state == st_manutencao);
    }
    {
        const event_t ev = { .sig = EV_RESET };
        fsm_dispatch(&catraca, &ev);
        assert(catraca.state == st_travada);
    }

    /* Parte 4: confirma pelo trace que so as transicoes REAIS entraram --
     * se isso falhar em 5 em vez de 4, um evento que deveria ser ignorado
     * esta, por engano, disparando fsm_tran. */
    assert(trace_count == 4);

    printf("test_catraca: OK\n");
    return 0;
}
