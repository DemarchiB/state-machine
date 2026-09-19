/* ------------------------------- ao_port.c -------------------------------- */
/**
 * @file ao_port.c
 * @brief Porta FreeRTOS do Active Object.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include "sm/ao.h"

#include "ao_port_api.h"

static void task_entry(void *arg);

ao_err_t ao_port_start(ao_t *const me, const ao_port_cfg_t *const cfg)
{
    ao_port_t *const p = &me->port;

    SM_ASSERT(cfg->queue_storage != NULL);
    SM_ASSERT(cfg->queue_len >= 1U);
    SM_ASSERT(cfg->stack != NULL);
    SM_ASSERT(cfg->priority < (UBaseType_t)configMAX_PRIORITIES);

    p->queue = xQueueCreateStatic(cfg->queue_len, (UBaseType_t)sizeof(ao_msg_t),
                                  (uint8_t *)cfg->queue_storage, &p->queue_cb);
    if (p->queue == NULL) {
        return AO_ERR_PORT;
    }

    p->task = xTaskCreateStatic(task_entry,
                                (cfg->name != NULL) ? cfg->name : "ao",
                                cfg->stack_depth, me, cfg->priority,
                                cfg->stack, &p->task_cb);
    if (p->task == NULL) {
        vQueueDelete(p->queue);
        p->queue = NULL;
        return AO_ERR_PORT;
    }
    return AO_OK;
}

ao_err_t ao_port_post(ao_t *const me, const ao_msg_t *const msg)
{
    /* Tempo de espera 0: postar nunca bloqueia quem posta -- inclusive a
     * propria task, que travaria esperando espaco numa fila que so ela
     * esvazia. */
    if (xQueueSendToBack(me->port.queue, msg, 0) != pdPASS) {
        return AO_ERR_QUEUE_FULL;
    }
    return AO_OK;
}

ao_err_t ao_port_post_from_isr(ao_t *const me, const ao_msg_t *const msg)
{
    BaseType_t woken = pdFALSE;
    ao_err_t err = AO_OK;

    if (xQueueSendToBackFromISR(me->port.queue, msg, &woken) != pdPASS) {
        err = AO_ERR_QUEUE_FULL;
    }
    portYIELD_FROM_ISR(woken);
    return err;
}

void ao_port_receive(ao_t *const me, ao_msg_t *const msg)
{
    /* portMAX_DELAY com INCLUDE_vTaskSuspend = 1 e espera sem limite; o
     * laco cobre a configuracao em que ele expira mesmo assim. */
    while (xQueueReceive(me->port.queue, msg, portMAX_DELAY) != pdPASS) {
    }
}

static void task_entry(void *arg)
{
    ao_run((ao_t *)arg);
    /* Depois de ao_stop: a task deixa de rodar. A fila continua existindo
     * (memoria da aplicacao), mas nada postado depois sera lido. Task do
     * FreeRTOS nao pode retornar. */
#if (INCLUDE_vTaskDelete == 1)
    vTaskDelete(NULL);
#else
    for (;;) {
        vTaskSuspend(NULL);
    }
#endif
}
