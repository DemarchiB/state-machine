/* ------------------------------- ao_port.h -------------------------------- */
/**
 * @file ao_port.h
 * @brief Porta FreeRTOS do Active Object: uma task e uma fila do kernel por
 *        objeto, ambas com alocacao estatica. Incluido por sm/ao.h -- nao
 *        inclua diretamente.
 * @copyright Bruno Luiz Demarchi (c) 2026
 *
 * Requer no FreeRTOSConfig.h: configSUPPORT_STATIC_ALLOCATION = 1 e
 * INCLUDE_vTaskSuspend = 1 (espera sem tempo limite em xQueueReceive).
 * Com INCLUDE_vTaskDelete = 1, a task e apagada apos ao_stop; sem ele, fica
 * suspensa para sempre.
 */

#ifndef AO_PORT_H
#define AO_PORT_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#if (configSUPPORT_STATIC_ALLOCATION != 1)
#error "state-machine/ao (porta freertos): configSUPPORT_STATIC_ALLOCATION precisa ser 1"
#endif
#if (INCLUDE_vTaskSuspend != 1)
#error "state-machine/ao (porta freertos): INCLUDE_vTaskSuspend precisa ser 1"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Parametros de ao_start nesta porta. Toda memoria e da aplicacao. */
typedef struct {
    ao_msg_t    *queue_storage; /**< Posicoes da fila; nao nulo. */
    UBaseType_t  queue_len;     /**< Numero de posicoes, >= 1. */
    StackType_t *stack;         /**< Pilha da task; nao nulo. */
    uint32_t     stack_depth;   /**< Tamanho da pilha, em StackType_t (nao em bytes). */
    UBaseType_t  priority;      /**< Prioridade da task, < configMAX_PRIORITIES. */
    const char  *name;          /**< Nome da task (depuracao); pode ser nulo. */
} ao_port_cfg_t;

/** Estado da porta dentro de ao_t. Privado. */
typedef struct {
    QueueHandle_t queue;
    StaticQueue_t queue_cb;
    TaskHandle_t  task;
    StaticTask_t  task_cb;
} ao_port_t;

#ifdef __cplusplus
}
#endif

#endif /* AO_PORT_H */
