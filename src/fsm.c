/* --------------------------------- fsm.c ---------------------------------- */
/**
 * @file fsm.c
 * @brief Maquina de estados finitos plana (FSM).
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include "sm/fsm.h"

void fsm_init(fsm_t *const me, fsm_state_fn initial_state)
{
    static const sm_event_t entry_ev = { SM_SIG_ENTRY };

    SM_ASSERT(me != NULL);
    SM_ASSERT(initial_state != NULL);

    me->state = initial_state;
    me->state(me, &entry_ev);
}

void fsm_dispatch(fsm_t *const me, const sm_event_t *const e)
{
    SM_ASSERT(me != NULL);
    SM_ASSERT(me->state != NULL);
    SM_ASSERT(e != NULL);
    SM_ASSERT(e->sig >= (uint16_t)SM_SIG_USER); /* reservados sao internos */

    me->state(me, e);
}
