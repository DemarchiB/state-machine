/* ----------------------------- catraca_hsm.c ------------------------------ */
/**
 * @file catraca_hsm.c
 * @brief Exemplo de hsm: catraca com falha segura hierarquica.
 * @copyright Bruno Luiz Demarchi (c) 2026
 *
 * Hierarquia:
 *
 *   hsm_top
 *   └── st_catraca              EV_FALHA_* -> st_falha_* correspondente
 *       ├── st_operacional      (inicial -> st_travada)
 *       │   ├── st_travada      entrada: fecha trava; EV_MOEDA -> st_liberada
 *       │   └── st_liberada     entrada: abre trava;  EV_EMPURRAR -> st_travada
 *       └── st_falha_segura     entrada: fecha trava; EV_RESET -> st_operacional
 *           ├── st_falha_sensor       entrada: registra a falha
 *           ├── st_falha_atuador      entrada: registra a falha
 *           └── st_falha_comunicacao  entrada: registra a falha
 *
 * O que a hierarquia economiza: as tres transicoes de falha estao escritas
 * uma vez (st_catraca), valendo para qualquer estado; fechar a trava e
 * ignorar EV_MOEDA/EV_EMPURRAR em falha, uma vez (st_falha_segura), valendo
 * para as tres causas. Uma quarta causa de falha e um estado filho com uma
 * acao de entrada e uma linha em st_catraca.
 *
 * Eventos nao listados num estado sobem ao pai; os que chegam a hsm_top
 * sao ignorados (ex.: EV_EMPURRAR em st_travada, EV_RESET em operacao).
 */

#include "catraca_hsm.h"

#include <stdio.h>

/* A funcao de estado recebe hsm_t*; o downcast vale porque hsm_t e o
 * primeiro membro de catraca_hsm_t. */
static catraca_hsm_t *self(hsm_t *const me)
{
    return (catraca_hsm_t *)me;
}

static void fecha_trava(hsm_t *const me)
{
    self(me)->trava_fechada = true;
    (void)printf("  acao: fecha trava\n");
}

static void abre_trava(hsm_t *const me)
{
    self(me)->trava_fechada = false;
    (void)printf("  acao: abre trava\n");
}

static void registra_falha(hsm_t *const me, const char *causa)
{
    self(me)->falhas++;
    (void)printf("  acao: registra falha de %s\n", causa);
}

void catraca_hsm_init(catraca_hsm_t *const me)
{
    me->trava_fechada = false;
    me->falhas        = 0U;
    hsm_init(&me->super, st_catraca);
}

hsm_status_t st_catraca(hsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_INIT:
        return hsm_tran(me, st_operacional);
    case EV_FALHA_SENSOR:
        return hsm_tran(me, st_falha_sensor);
    case EV_FALHA_ATUADOR:
        return hsm_tran(me, st_falha_atuador);
    case EV_FALHA_COMUNICACAO:
        return hsm_tran(me, st_falha_comunicacao);
    default:
        return hsm_super(me, hsm_top);
    }
}

hsm_status_t st_operacional(hsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_INIT:
        return hsm_tran(me, st_travada);
    default:
        return hsm_super(me, st_catraca);
    }
}

hsm_status_t st_travada(hsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY:
        fecha_trava(me);
        return HSM_HANDLED;
    case EV_MOEDA:
        return hsm_tran(me, st_liberada);
    default:
        return hsm_super(me, st_operacional);
    }
}

hsm_status_t st_liberada(hsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY:
        abre_trava(me);
        return HSM_HANDLED;
    case EV_EMPURRAR:
        return hsm_tran(me, st_travada);
    case EV_MOEDA:
        return HSM_HANDLED; /* moeda perdida: ignorada de proposito */
    default:
        return hsm_super(me, st_operacional);
    }
}

hsm_status_t st_falha_segura(hsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY:
        fecha_trava(me);
        return HSM_HANDLED;
    case EV_RESET:
        return hsm_tran(me, st_operacional);
    case EV_MOEDA:
    case EV_EMPURRAR:
        return HSM_HANDLED; /* em falha, a catraca nao reage a uso */
    default:
        return hsm_super(me, st_catraca);
    }
}

hsm_status_t st_falha_sensor(hsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY:
        registra_falha(me, "sensor");
        return HSM_HANDLED;
    default:
        return hsm_super(me, st_falha_segura);
    }
}

hsm_status_t st_falha_atuador(hsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY:
        registra_falha(me, "atuador");
        return HSM_HANDLED;
    default:
        return hsm_super(me, st_falha_segura);
    }
}

hsm_status_t st_falha_comunicacao(hsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY:
        registra_falha(me, "comunicacao");
        return HSM_HANDLED;
    default:
        return hsm_super(me, st_falha_segura);
    }
}
