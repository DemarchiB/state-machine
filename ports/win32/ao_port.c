/* ------------------------------- ao_port.c -------------------------------- */
/**
 * @file ao_port.c
 * @brief Porta Win32 do Active Object.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include "sm/ao.h"

#include <string.h>

#include "ao_port_api.h"

static DWORD WINAPI thread_entry(LPVOID arg);
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

    InitializeCriticalSection(&p->lock);
    InitializeConditionVariable(&p->not_empty);
    p->thread = CreateThread(NULL, 0U, thread_entry, me, 0U, NULL);
    if (p->thread == NULL) {
        DeleteCriticalSection(&p->lock);
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

    EnterCriticalSection(&p->lock);
    while (p->count == 0U) {
        (void)SleepConditionVariableCS(&p->not_empty, &p->lock, INFINITE);
    }
    (void)memcpy(msg, &p->buf[p->head], sizeof(*msg));
    p->head = (p->head + 1U) % p->len;
    p->count--;
    LeaveCriticalSection(&p->lock);
}

void ao_join(ao_t *const me)
{
    SM_ASSERT(me != NULL);
    SM_ASSERT(me->started);

    (void)WaitForSingleObject(me->port.thread, INFINITE);
    (void)CloseHandle(me->port.thread);
    DeleteCriticalSection(&me->port.lock);
    me->started = false;
}

static DWORD WINAPI thread_entry(LPVOID arg)
{
    ao_run((ao_t *)arg);
    return 0U;
}

static ao_err_t push(ao_t *const me, const ao_msg_t *const msg)
{
    ao_port_t *const p = &me->port;
    ao_err_t err = AO_ERR_QUEUE_FULL;

    EnterCriticalSection(&p->lock);
    if (p->count < p->len) {
        (void)memcpy(&p->buf[(p->head + p->count) % p->len], msg, sizeof(*msg));
        p->count++;
        WakeConditionVariable(&p->not_empty);
        err = AO_OK;
    }
    LeaveCriticalSection(&p->lock);
    return err;
}
