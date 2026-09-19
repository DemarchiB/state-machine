/* --------------------------------- main.c --------------------------------- */
/**
 * @file main.c
 * @brief Exemplo de Active Object (portas de host): a mesma catraca do
 *        exemplo fsm_catraca, sem nenhuma alteracao, agora rodando na
 *        propria thread e recebendo eventos postados pela thread principal.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include <stdio.h>
#include <stdlib.h>

#include "sm/ao.h"
#include "catraca.h"

/** Active Object da catraca: ao_t primeiro, a maquina depois. */
typedef struct {
    ao_t  super;
    fsm_t sm;
} catraca_ao_t;

static void catraca_setup(ao_t *const me)
{
    fsm_init(&((catraca_ao_t *)me)->sm, st_travada);
}

static void catraca_handler(ao_t *const me, const sm_event_t *const e)
{
    fsm_dispatch(&((catraca_ao_t *)me)->sm, e);
}

static catraca_ao_t catraca;
static ao_msg_t     catraca_fila[8];

static void posta(uint16_t sig)
{
    sm_event_t ev;

    ev.sig = sig;

    if (ao_post(&catraca.super, &ev, sizeof(ev)) != AO_OK) {
        (void)printf("fila cheia: evento %u descartado\n", (unsigned)sig);
    }
}

int main(void)
{
    const ao_port_cfg_t cfg = { catraca_fila, sizeof(catraca_fila) / sizeof(catraca_fila[0]) };

    ao_init(&catraca.super, catraca_setup, catraca_handler);
    if (ao_start(&catraca.super, &cfg) != AO_OK) {
        (void)printf("falha ao iniciar o Active Object\n");
        return EXIT_FAILURE;
    }

    /* A thread principal so posta; quem despacha e a thread do objeto. */
    posta(EV_MOEDA);
    posta(EV_EMPURRAR);
    posta(EV_FALHA_SENSOR);
    posta(EV_RESET);

    /* Eventos postados antes do pedido de parada ainda sao despachados. A
     * fila tem 8 posicoes e so 4 foram usadas: o pedido sempre cabe. */
    if (ao_stop(&catraca.super) != AO_OK) {
        (void)printf("fila cheia: pedido de parada descartado\n");
        return EXIT_FAILURE;
    }
    ao_join(&catraca.super);

    (void)printf("estado final: %s\n",
                 fsm_is_in(&catraca.sm, st_travada) ? "st_travada" : "outro");
    return EXIT_SUCCESS;
}
