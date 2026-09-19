/* ------------------------------- ao_port.c -------------------------------- */
/**
 * @file ao_port.c
 * @brief Porta POSIX do Active Object.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include "sm/ao.h"

#include <string.h>

#include "ao_port_api.h"

static void *thread_entry(void *arg);
static ao_err_t push(ao_t *const me, const ao_msg_t *const msg);

ao_err_t ao_port_start(ao_t *const me, const ao_port_cfg_t *const cfg)
{
    ao_port_t *const p = &me->port;

    SM_ASSERT(cfg->queue_storage != NULL);
    SM_ASSERT(cfg->queue_len >= 1U);

    p->buf   = cfg->queue_storage;
    p->len   = cfg->queue_len;
    p->head  = 0U;
    p->count = 0U;

    if (pthread_mutex_init(&p->lock, NULL) != 0) {
        return AO_ERR_PORT;
    }
    if (pthread_cond_init(&p->not_empty, NULL) != 0) {
        (void)pthread_mutex_destroy(&p->lock);
        return AO_ERR_PORT;
    }
    if (pthread_create(&p->thread, NULL, thread_entry, me) != 0) {
        (void)pthread_cond_destroy(&p->not_empty);
        (void)pthread_mutex_destroy(&p->lock);
        return AO_ERR_PORT;
    }
    return AO_OK;
}

ao_err_t ao_port_post(ao_t *const me, const ao_msg_t *const msg)
{
    return push(me, msg);
}

ao_err_t ao_port_post_from_isr(ao_t *const me, const ao_msg_t *const msg)
{
    return push(me, msg); /* host nao tem interrupcao */
}

void ao_port_receive(ao_t *const me, ao_msg_t *const msg)
{
    ao_port_t *const p = &me->port;

    (void)pthread_mutex_lock(&p->lock);
    while (p->count == 0U) {
        (void)pthread_cond_wait(&p->not_empty, &p->lock);
    }
    (void)memcpy(msg, &p->buf[p->head], sizeof(*msg));
    p->head = (p->head + 1U) % p->len;
    p->count--;
    (void)pthread_mutex_unlock(&p->lock);
}

void ao_join(ao_t *const me)
{
    SM_ASSERT(me != NULL);
    SM_ASSERT(me->started);

    (void)pthread_join(me->port.thread, NULL);
    (void)pthread_cond_destroy(&me->port.not_empty);
    (void)pthread_mutex_destroy(&me->port.lock);
    me->started = false;
}

static void *thread_entry(void *arg)
{
    ao_run((ao_t *)arg);
    return NULL;
}

static ao_err_t push(ao_t *const me, const ao_msg_t *const msg)
{
    ao_port_t *const p = &me->port;
    ao_err_t err = AO_ERR_QUEUE_FULL;

    (void)pthread_mutex_lock(&p->lock);
    if (p->count < p->len) {
        (void)memcpy(&p->buf[(p->head + p->count) % p->len], msg, sizeof(*msg));
        p->count++;
        (void)pthread_cond_signal(&p->not_empty);
        err = AO_OK;
    }
    (void)pthread_mutex_unlock(&p->lock);
    return err;
}
