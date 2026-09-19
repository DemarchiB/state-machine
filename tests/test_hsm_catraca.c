/* --------------------------- test_hsm_catraca.c --------------------------- */
/**
 * @file test_hsm_catraca.c
 * @brief Teste em host do exemplo da catraca hierarquica: estado ativo,
 *        saida fisica simulada e contagem de falhas a cada passo.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include <stdio.h>

#include "catraca_hsm.h"
#include "check.h"

static void envia(catraca_hsm_t *const me, uint16_t sig)
{
    sm_event_t ev;

    ev.sig = sig;
    hsm_dispatch(&me->super, &ev);
}

int main(void)
{
    catraca_hsm_t c;

    /* Iniciais encadeadas: st_catraca -> st_operacional -> st_travada. */
    catraca_hsm_init(&c);
    CHECK(hsm_state(&c.super) == st_travada);
    CHECK(hsm_is_in(&c.super, st_operacional));
    CHECK(c.trava_fechada);

    envia(&c, EV_EMPURRAR); /* sobe ate hsm_top: ignorado */
    CHECK(hsm_state(&c.super) == st_travada);

    envia(&c, EV_MOEDA);
    CHECK(hsm_state(&c.super) == st_liberada);
    CHECK(!c.trava_fechada);

    /* A transicao de falha, escrita uma vez em st_catraca, vale a partir de
     * st_liberada; a entrada de st_falha_segura fecha a trava. */
    envia(&c, EV_FALHA_ATUADOR);
    CHECK(hsm_state(&c.super) == st_falha_atuador);
    CHECK(hsm_is_in(&c.super, st_falha_segura));
    CHECK(!hsm_is_in(&c.super, st_operacional));
    CHECK(c.trava_fechada);
    CHECK(c.falhas == 1U);

    /* Regra comum das tres falhas, escrita uma vez no pai. */
    envia(&c, EV_MOEDA);
    envia(&c, EV_EMPURRAR);
    CHECK(hsm_state(&c.super) == st_falha_atuador);
    CHECK(c.trava_fechada);

    /* Troca de causa: quem trata e st_catraca, entao st_falha_segura sai e
     * entra de novo (a trava e fechada outra vez) antes da nova causa. */
    envia(&c, EV_FALHA_COMUNICACAO);
    CHECK(hsm_state(&c.super) == st_falha_comunicacao);
    CHECK(c.falhas == 2U);

    /* Reset para o composto: a inicial leva a st_travada. */
    envia(&c, EV_RESET);
    CHECK(hsm_state(&c.super) == st_travada);
    CHECK(c.trava_fechada);

    envia(&c, EV_RESET); /* em operacao: ignorado */
    CHECK(hsm_state(&c.super) == st_travada);

    envia(&c, EV_FALHA_SENSOR);
    CHECK(hsm_state(&c.super) == st_falha_sensor);
    CHECK(c.falhas == 3U);

    (void)printf("test_hsm_catraca: OK\n");
    return 0;
}
