/* ------------------------------ catraca.c --------------------------------- */
/**
 * @file catraca.c
 * @brief Exemplo de uso da biblioteca fsm: catraca de tres estados, com
 *        manutencao e reset. Roda em host, sem hardware -- as acoes sao
 *        so printf.
 */

#include <stdio.h>

#include "fsm.h"

/* Sinais da aplicacao comecam em SIG_USER (fsm.h) -- os reservados
 * (SIG_ENTRY/SIG_EXIT) ficam abaixo, pra biblioteca. */
enum {
    EV_MOEDA = SIG_USER,
    EV_EMPURRAR,
    EV_FALHA_SENSOR,
    EV_RESET
};

static void travar(void)  { printf("Acao: fecha trava\n"); }
static void liberar(void) { printf("Acao: abre trava\n"); }

static void st_travada(fsm_t *me, const event_t *e);
static void st_liberada(fsm_t *me, const event_t *e);
static void st_manutencao(fsm_t *me, const event_t *e);

/* >>> SUA PARTE <<<
 * Cada funcao de estado trata SIG_ENTRY/SIG_EXIT alem dos eventos de
 * aplicacao -- travar()/liberar() saem do meio da logica de transicao e
 * viram reacao ao SIG_ENTRY do estado certo. Use fsm_tran para trocar de
 * estado -- nunca atribua me->estado diretamente, ou SIG_ENTRY/SIG_EXIT
 * nao disparam.
 *
 * Tabela de referencia (igual a fase-0, so que agora com entry/exit):
 *   TRAVADA  + EV_MOEDA        -> LIBERADA
 *   TRAVADA  + EV_EMPURRAR     -> ignorado
 *   LIBERADA + EV_EMPURRAR     -> TRAVADA
 *   LIBERADA + EV_MOEDA        -> ignorado (moeda perdida)
 *   qualquer + EV_FALHA_SENSOR -> MANUTENCAO
 *   MANUTENCAO + EV_RESET      -> TRAVADA
 *   MANUTENCAO + outro evento  -> ignorado
 */
static void st_travada(fsm_t *me, const event_t *e)
{
    (void)me;
    (void)e;
}

static void st_liberada(fsm_t *me, const event_t *e)
{
    (void)me;
    (void)e;
}

static void st_manutencao(fsm_t *me, const event_t *e)
{
    (void)me;
    (void)e;
}

int main(void)
{
    fsm_t catraca;

    fsm_init(&catraca, st_travada);

    /* opcional: despache uma sequencia de eventos de teste aqui e confira
       pelo printf se bate com o esperado, antes de escrever tests/. */

    return 0;
}
