/* ------------------------------- catraca.c -------------------------------- */
/**
 * @file catraca.c
 * @brief Exemplo de fsm: catraca de controle de acesso. As acoes sao
 *        printf -- roda no host, sem hardware.
 * @copyright Bruno Luiz Demarchi (c) 2026
 *
 * Tabela de transicao (todo par estado x evento decidido):
 *
 * | Estado       | EV_MOEDA            | EV_EMPURRAR  | EV_FALHA_SENSOR | EV_RESET    |
 * |--------------|---------------------|--------------|-----------------|-------------|
 * | st_travada   | -> st_liberada      | ignorado     | -> st_manutencao| ignorado    |
 * | st_liberada  | ignorado (perdida)  | -> st_travada| -> st_manutencao| ignorado    |
 * | st_manutencao| ignorado            | ignorado     | ignorado        | -> st_travada|
 *
 * Acoes de entrada: st_travada e st_manutencao fecham a trava; st_liberada
 * a abre. Como a acao mora no SM_SIG_ENTRY, qualquer caminho que chegue ao
 * estado executa a acao -- nenhuma transicao precisa lembrar dela.
 */

#include "catraca.h"

#include <stdio.h>

static void fecha_trava(void) { (void)printf("  acao: fecha trava\n"); }
static void abre_trava(void)  { (void)printf("  acao: abre trava\n"); }

void st_travada(fsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY:
        fecha_trava();
        break;
    case EV_MOEDA:
        fsm_tran(me, st_liberada);
        break;
    case EV_FALHA_SENSOR:
        fsm_tran(me, st_manutencao);
        break;
    default:
        break; /* EV_EMPURRAR, EV_RESET: ignorados */
    }
}

void st_liberada(fsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY:
        abre_trava();
        break;
    case EV_EMPURRAR:
        fsm_tran(me, st_travada);
        break;
    case EV_FALHA_SENSOR:
        fsm_tran(me, st_manutencao);
        break;
    default:
        break; /* EV_MOEDA (moeda perdida), EV_RESET: ignorados */
    }
}

void st_manutencao(fsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY:
        fecha_trava();
        break;
    case EV_RESET:
        fsm_tran(me, st_travada);
        break;
    default:
        break; /* EV_MOEDA, EV_EMPURRAR, EV_FALHA_SENSOR: ignorados */
    }
}
