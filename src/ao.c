/* ---------------------------------- ao.c ---------------------------------- */
/**
 * @file ao.c
 * @brief Active Object: parte independente de porta (copia de eventos e
 *        laco de despacho).
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include "sm/ao.h"

#include <string.h>

#include "ao_port_api.h"

/* Private enums -------------------------------------------------------------*/

enum {
    MSG_EVENT = 0, /**< Posicao carrega um evento para o handler. */
    MSG_STOP       /**< Pedido de parada; encerra ao_run. */
};

SM_STATIC_ASSERT(AO_EVENT_SIZE_MAX >= sizeof(sm_event_t), ao_event_size_max);

/* Class' private operations (methods) declaration ---------------------------*/

static void make_event_msg(ao_msg_t *const msg, const sm_event_t *const e,
                           size_t size_bytes);

/* Class' public operations (methods) definition -----------------------------*/

void ao_init(ao_t *const me, ao_setup_fn setup, ao_handler_fn handler)
{
    SM_ASSERT(me != NULL);
    SM_ASSERT(setup != NULL);
    SM_ASSERT(handler != NULL);

    (void)memset(me, 0, sizeof(*me));
    me->setup   = setup;
    me->handler = handler;
    me->started = false;
}

ao_err_t ao_start(ao_t *const me, const ao_port_cfg_t *const cfg)
{
    ao_err_t err;

    SM_ASSERT(me != NULL);
    SM_ASSERT(cfg != NULL);
    SM_ASSERT(me->setup != NULL);   /* ao_init nao foi chamado */
    SM_ASSERT(!me->started);

    /* Marcado antes de criar a task: ela pode rodar (e postar para si
     * mesma no setup) antes de ao_port_start retornar. */
    me->started = true;
    err = ao_port_start(me, cfg);
    if (err != AO_OK) {
        me->started = false;
    }
    return err;
}

ao_err_t ao_post(ao_t *const me, const sm_event_t *const e, size_t size_bytes)
{
    ao_msg_t msg;

    SM_ASSERT(me != NULL);
    SM_ASSERT(me->started);
    make_event_msg(&msg, e, size_bytes);
    return ao_port_post(me, &msg);
}

ao_err_t ao_post_from_isr(ao_t *const me, const sm_event_t *const e,
                          size_t size_bytes)
{
    ao_msg_t msg;

    SM_ASSERT(me != NULL);
    SM_ASSERT(me->started);
    make_event_msg(&msg, e, size_bytes);
    return ao_port_post_from_isr(me, &msg);
}

ao_err_t ao_stop(ao_t *const me)
{
    ao_msg_t msg;

    SM_ASSERT(me != NULL);
    SM_ASSERT(me->started);

    (void)memset(&msg, 0, sizeof(msg));
    msg.kind = (uint8_t)MSG_STOP;
    return ao_port_post(me, &msg);
}

void ao_run(ao_t *const me)
{
    ao_msg_t msg;

    SM_ASSERT(me != NULL);

    me->setup(me);
    for (;;) {
        ao_port_receive(me, &msg);
        if (msg.kind == (uint8_t)MSG_STOP) {
            break;
        }
        me->handler(me, &msg.storage.event);
    }
}

/* Class' private operations (methods) definition ----------------------------*/

static void make_event_msg(ao_msg_t *const msg, const sm_event_t *const e,
                           size_t size_bytes)
{
    SM_ASSERT(e != NULL);
    SM_ASSERT(e->sig >= (uint16_t)SM_SIG_USER); /* reservados sao internos */
    SM_ASSERT(size_bytes >= sizeof(sm_event_t));
    SM_ASSERT(size_bytes <= (size_t)AO_EVENT_SIZE_MAX); /* aumente o maximo */

    /* Zera antes para que a parte nao usada da posicao nunca carregue lixo
     * de pilha para a fila. */
    (void)memset(msg, 0, sizeof(*msg));
    msg->kind = (uint8_t)MSG_EVENT;
    (void)memcpy(msg->storage.bytes, e, size_bytes);
}
