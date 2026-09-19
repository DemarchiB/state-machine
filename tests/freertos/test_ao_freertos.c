/* --------------------------- test_ao_freertos.c --------------------------- */
/**
 * @file test_ao_freertos.c
 * @brief Teste da porta freertos no simulador POSIX do FreeRTOS: a catraca
 *        do exemplo fsm_catraca como Active Object, alimentada por uma task
 *        de teste de prioridade menor, e o caso de fila cheia.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include "sm/ao.h"

#include <stdio.h>

#include "catraca.h"
#include "check.h"

#define STACK_WORDS 1024U
#define PRIO_TESTE  1U
#define PRIO_AO     2U

void vAssertCalled(const char *file, unsigned long line)
{
    (void)fprintf(stderr, "configASSERT: %s:%lu\n", file, line);
    exit(EXIT_FAILURE);
}

/* --- trace da catraca -------------------------------------------------------*/

#define TRACE_MAX 16

static fsm_state_fn trace_para[TRACE_MAX];
static size_t       trace_len;

void fsm_trace(fsm_state_fn source, fsm_state_fn target)
{
    (void)source;
    CHECK(trace_len < TRACE_MAX);
    trace_para[trace_len] = target;
    trace_len++;
}

/* --- AO da catraca -----------------------------------------------------------*/

typedef struct {
    ao_t              super;
    fsm_t             sm;
    volatile uint32_t despachados; /* lido pela task de teste */
} catraca_ao_t;

static void catraca_setup(ao_t *const me)
{
    fsm_init(&((catraca_ao_t *)me)->sm, st_travada);
}

static void catraca_handler(ao_t *const me, const sm_event_t *const e)
{
    catraca_ao_t *const self = (catraca_ao_t *)me;

    fsm_dispatch(&self->sm, e);
    self->despachados++;
}

static catraca_ao_t catraca;
static ao_msg_t     catraca_fila[8];
static StackType_t  catraca_pilha[STACK_WORDS];

/* --- AO de fila cheia ---------------------------------------------------------*/

#define CHEIO_LEN 3

typedef struct {
    ao_t              super;
    ao_err_t          resultados[CHEIO_LEN + 1];
    volatile uint32_t despachados;
} cheio_ao_t;

static void cheio_setup(ao_t *const me)
{
    cheio_ao_t *const self = (cheio_ao_t *)me;
    const sm_event_t ev = { SM_SIG_USER };
    size_t i;

    for (i = 0U; i < (size_t)(CHEIO_LEN + 1); i++) {
        self->resultados[i] = ao_post(me, &ev, sizeof(ev));
    }
}

static void cheio_handler(ao_t *const me, const sm_event_t *const e)
{
    (void)e;
    ((cheio_ao_t *)me)->despachados++;
}

static cheio_ao_t  cheio;
static ao_msg_t    cheio_fila[CHEIO_LEN];
static StackType_t cheio_pilha[STACK_WORDS];

/* --- task de teste -------------------------------------------------------------*/

static StackType_t  teste_pilha[STACK_WORDS];
static StaticTask_t teste_tcb;

static void espera(volatile uint32_t *const contador, uint32_t alvo)
{
    uint32_t ticks;

    for (ticks = 0U; (ticks < 1000U) && (*contador < alvo); ticks++) {
        vTaskDelay(1);
    }
}

static void tarefa_teste(void *arg)
{
    const uint16_t sequencia[] = {
        EV_MOEDA, EV_EMPURRAR, EV_EMPURRAR, EV_FALHA_SENSOR, EV_RESET
    };
    size_t i;

    (void)arg;

    for (i = 0U; i < 5U; i++) {
        const sm_event_t ev = { sequencia[i] };
        CHECK(ao_post(&catraca.super, &ev, sizeof(ev)) == AO_OK);
    }
    espera(&catraca.despachados, 5U);
    CHECK(catraca.despachados == 5U);
    CHECK(fsm_is_in(&catraca.sm, st_travada));
    CHECK(trace_len == 4U);
    CHECK(trace_para[0] == st_liberada);
    CHECK(trace_para[1] == st_travada);
    CHECK(trace_para[2] == st_manutencao);
    CHECK(trace_para[3] == st_travada);
    CHECK(ao_stop(&catraca.super) == AO_OK);

    espera(&cheio.despachados, (uint32_t)CHEIO_LEN);
    for (i = 0U; i < (size_t)CHEIO_LEN; i++) {
        CHECK(cheio.resultados[i] == AO_OK);
    }
    CHECK(cheio.resultados[CHEIO_LEN] == AO_ERR_QUEUE_FULL);
    CHECK(cheio.despachados == (uint32_t)CHEIO_LEN);

    (void)printf("test_ao_freertos: OK\n");
    exit(EXIT_SUCCESS);
}

int main(void)
{
    const ao_port_cfg_t cfg_catraca = {
        catraca_fila, 8U, catraca_pilha, STACK_WORDS, PRIO_AO, "catraca"
    };
    const ao_port_cfg_t cfg_cheio = {
        cheio_fila, (UBaseType_t)CHEIO_LEN, cheio_pilha, STACK_WORDS, PRIO_AO, "cheio"
    };

    ao_init(&catraca.super, catraca_setup, catraca_handler);
    ao_init(&cheio.super, cheio_setup, cheio_handler);
    CHECK(ao_start(&catraca.super, &cfg_catraca) == AO_OK);
    CHECK(ao_start(&cheio.super, &cfg_cheio) == AO_OK);
    CHECK(xTaskCreateStatic(tarefa_teste, "teste", STACK_WORDS, NULL, PRIO_TESTE,
                            teste_pilha, &teste_tcb) != NULL);

    vTaskStartScheduler();
    return EXIT_FAILURE; /* so chega aqui se o escalonador nao iniciar */
}
