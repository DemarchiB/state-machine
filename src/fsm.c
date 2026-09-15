/* --------------------------------- fsm.c ---------------------------------- */
/**
 * @file fsm.c
 * @brief Implementacao da biblioteca de maquinas de estado finitas (FSM).
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

/* Private includes ----------------------------------------------------------*/
#include "fsm.h"

/* Class' public operations (methods) definition -----------------------------*/

void fsm_init(fsm_t *const me, fsm_state_fn initial_state)
{
    static const event_t ENTRY_EV = { .sig = SIG_ENTRY };

    FSM_ASSERT(me != NULL);
    FSM_ASSERT(initial_state != NULL);

    me->state = initial_state;
    me->state(me, &ENTRY_EV);   /* dispara a acao de entrada do estado inicial */
}

void fsm_dispatch(fsm_t *const me, const event_t *const e)
{
    FSM_ASSERT(me != NULL);
    FSM_ASSERT(e != NULL);

    me->state(me, e);
}

/* fsm_tran fica em fsm.h, como static inline -- ver o comentario la pelo
 * porque da escolha. Nada mais entra aqui por enquanto. */
