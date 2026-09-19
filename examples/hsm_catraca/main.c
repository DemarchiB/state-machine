/* --------------------------------- main.c --------------------------------- */
/**
 * @file main.c
 * @brief Exemplo de hsm: despacha uma sequencia de eventos para a catraca
 *        hierarquica e imprime as acoes.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include <stdio.h>

#include "catraca_hsm.h"

static void envia(catraca_hsm_t *const catraca, uint16_t sig, const char *nome)
{
    sm_event_t ev;

    ev.sig = sig;

    (void)printf("%s\n", nome);
    hsm_dispatch(&catraca->super, &ev);
}

int main(void)
{
    catraca_hsm_t catraca;

    (void)printf("catraca_hsm_init\n");
    catraca_hsm_init(&catraca);

    envia(&catraca, EV_MOEDA, "EV_MOEDA");
    envia(&catraca, EV_FALHA_ATUADOR, "EV_FALHA_ATUADOR (liberada -> falha)");
    envia(&catraca, EV_MOEDA, "EV_MOEDA (ignorado em falha)");
    envia(&catraca, EV_FALHA_SENSOR, "EV_FALHA_SENSOR (troca a causa)");
    envia(&catraca, EV_RESET, "EV_RESET");

    (void)printf("falhas registradas: %u\n", (unsigned)catraca.falhas);
    return 0;
}
