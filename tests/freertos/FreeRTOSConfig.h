/* ---------------------------- FreeRTOSConfig.h ---------------------------- */
/**
 * @file FreeRTOSConfig.h
 * @brief Configuracao minima do FreeRTOS para o teste da porta freertos no
 *        simulador POSIX. So teste; nao e referencia para firmware.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0
#define configTICK_RATE_HZ                      1000
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                256
#define configMAX_TASK_NAME_LEN                 12
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_TRACE_FACILITY                0
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                4
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE
#define configUSE_CO_ROUTINES                   0
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_MALLOC_FAILED_HOOK            0
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configKERNEL_PROVIDED_STATIC_MEMORY     1
#define configTOTAL_HEAP_SIZE                   (64U * 1024U)

#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1

#define configASSERT(x)                                                    \
    do {                                                                   \
        if (!(x)) {                                                        \
            vAssertCalled(__FILE__, __LINE__);                             \
        }                                                                  \
    } while (0)

void vAssertCalled(const char *file, unsigned long line);

#endif /* FREERTOS_CONFIG_H */
