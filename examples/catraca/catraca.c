/* ------------------------------ catraca.c --------------------------------- */
/**
 * @file catraca.c
 * @brief Exemplo de uso da biblioteca fsm: catraca de tres estados, com
 *        manutencao e reset. Roda em host, sem hardware -- as acoes sao
 *        so printf.
 */

#include <stdio.h>

#include "catraca.h"

static void travar(void)  { printf("Acao: fecha trava\n"); }
static void liberar(void) { printf("Acao: abre trava\n"); }

/* Tabela de referencia (igual a fase-0, so que agora com entry/exit):
 *   TRAVADA  + EV_MOEDA        -> LIBERADA
 *   TRAVADA  + EV_EMPURRAR     -> ignorado
 *   LIBERADA + EV_EMPURRAR     -> TRAVADA
 *   LIBERADA + EV_MOEDA        -> ignorado (moeda perdida)
 *   qualquer + EV_FALHA_SENSOR -> MANUTENCAO
 *   MANUTENCAO + EV_RESET      -> TRAVADA
 *   MANUTENCAO + outro evento  -> ignorado
 */
void st_travada(fsm_t *me, const event_t *e)
{
    switch (e->sig) {
        case SIG_ENTRY:
            travar();
            break;
        case EV_MOEDA:
            fsm_tran(me, st_liberada);
            break;
        case EV_FALHA_SENSOR:
            fsm_tran(me, st_manutencao);
            break;
        default:
            break;
    }
}

void st_liberada(fsm_t *me, const event_t *e)
{
    switch (e->sig) {
        case SIG_ENTRY:
            liberar();
            break;
        case EV_EMPURRAR:
            fsm_tran(me, st_travada);
            break;
        case EV_FALHA_SENSOR:
            fsm_tran(me, st_manutencao);
            break;
        default:
            break;
    }
}

void st_manutencao(fsm_t *me, const event_t *e)
{
    switch (e->sig) {
        case SIG_ENTRY:
            travar();
            break;
        case EV_RESET:
            fsm_tran(me, st_travada);
            break;
        default:
            break;
    }
}
