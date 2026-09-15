/* --------------------------------- main.c ---------------------------------- */
/**
 * @file main.c
 * @brief Ponto de entrada do exemplo catraca -- so o driver; a maquina em
 *        si (sinais e funcoes de estado) mora em catraca.h/catraca.c, pra
 *        poder ser reaproveitada por tests/test_catraca.c tambem.
 */

#include "catraca.h"

int main(void)
{
    fsm_t catraca;

    fsm_init(&catraca, st_travada);

    event_t event;
    event.sig = EV_MOEDA;
    fsm_dispatch(&catraca, &event);
    event.sig = EV_EMPURRAR;
    fsm_dispatch(&catraca, &event);
    event.sig = EV_EMPURRAR;
    fsm_dispatch(&catraca, &event);
    event.sig = EV_FALHA_SENSOR;
    fsm_dispatch(&catraca, &event);
    event.sig = EV_RESET;
    fsm_dispatch(&catraca, &event);

    return 0;
}
