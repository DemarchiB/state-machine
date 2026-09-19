/* --------------------------------- main.c --------------------------------- */
/**
 * @file main.c
 * @brief Exemplo de fsm: despacha uma sequencia de eventos para a catraca,
 *        de forma sincrona, e imprime as acoes.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include <stdio.h>

#include "catraca.h"

static void envia(fsm_t *const catraca, uint16_t sig, const char *nome)
{
    sm_event_t ev;

    ev.sig = sig;

    (void)printf("%s\n", nome);
    fsm_dispatch(catraca, &ev);
}

int main(void)
{
    fsm_t catraca;

    (void)printf("fsm_init\n");
    fsm_init(&catraca, st_travada);

    envia(&catraca, EV_MOEDA, "EV_MOEDA");
    envia(&catraca, EV_EMPURRAR, "EV_EMPURRAR");
    envia(&catraca, EV_EMPURRAR, "EV_EMPURRAR (ignorado: ja travada)");
    envia(&catraca, EV_FALHA_SENSOR, "EV_FALHA_SENSOR");
    envia(&catraca, EV_RESET, "EV_RESET");

    return 0;
}
