/* -------------------------------- test_ao.c ------------------------------- */
/**
 * @file test_ao.c
 * @brief Teste em host do Active Object (portas posix e win32), com threads
 *        reais e sem nenhuma primitiva de sistema no proprio teste:
 *
 *  1. Tres contextos: a thread principal posta comandos, com payload, para
 *     o AO "sensor", que os converte e posta para o AO da catraca (a fsm do
 *     exemplo fsm_catraca, sem alteracao). O trace da catraca prova que cada
 *     evento foi despachado uma vez, em ordem, e que o pedido de parada so
 *     encerra depois dos eventos postados antes dele.
 *  2. Fila cheia, de forma deterministica: o setup de um AO posta para a
 *     propria fila -- enquanto ele roda ninguem consome -- e a posicao
 *     excedente volta AO_ERR_QUEUE_FULL.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#include "sm/ao.h"

#include <stdio.h>

#include "catraca.h"
#include "check.h"

/* --- trace da catraca (escrito so pela thread do AO da catraca; lido pela
 *     principal depois de ao_join, que garante a visibilidade) ------------*/

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

/* --- AO da catraca ---------------------------------------------------------*/

typedef struct {
    ao_t     super;
    fsm_t    sm;
    uint32_t despachados;
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
static ao_msg_t     catraca_fila[8]; /* >= 5 eventos + parada: nunca enche */

/* --- AO sensor: evento com payload -> evento simples para a catraca -------*/

enum { EV_COMANDO = EV_RESET + 1 };

typedef struct {
    sm_event_t super;      /**< Sempre o primeiro membro. */
    uint16_t   para_catraca; /**< Sinal a repassar. */
    uint32_t   sequencia;  /**< Numero do comando, para conferir a ordem. */
} ev_comando_t;

typedef struct {
    ao_t     super;
    uint32_t proxima_sequencia;
    uint32_t fora_de_ordem;
} sensor_ao_t;

static void sensor_setup(ao_t *const me)
{
    (void)me;
}

static void sensor_handler(ao_t *const me, const sm_event_t *const e)
{
    sensor_ao_t *const self = (sensor_ao_t *)me;
    const ev_comando_t *const cmd = (const ev_comando_t *)e;
    sm_event_t repasse;

    CHECK(e->sig == EV_COMANDO);
    if (cmd->sequencia != self->proxima_sequencia) {
        self->fora_de_ordem++;
    }
    self->proxima_sequencia++;

    repasse.sig = cmd->para_catraca;
    CHECK(ao_post(&catraca.super, &repasse, sizeof(repasse)) == AO_OK);
}

static sensor_ao_t sensor;
static ao_msg_t    sensor_fila[8];

/* --- AO para o teste de fila cheia ----------------------------------------*/

#define CHEIO_LEN 3

typedef struct {
    ao_t     super;
    ao_err_t resultados[CHEIO_LEN + 1];
    uint32_t despachados;
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
    cheio_ao_t *const self = (cheio_ao_t *)me;

    (void)e;
    self->despachados++;
    /* O proprio AO pede a parada depois do ultimo evento aceito: se a
     * thread principal postasse a parada, ela poderia ocupar uma posicao
     * antes do setup e tornar o resultado dependente de escalonamento. */
    if (self->despachados == (uint32_t)CHEIO_LEN) {
        CHECK(ao_stop(me) == AO_OK);
    }
}

static cheio_ao_t cheio;
static ao_msg_t   cheio_fila[CHEIO_LEN];

/* --- casos -----------------------------------------------------------------*/

static void posta_comando(uint16_t para_catraca, uint32_t sequencia)
{
    ev_comando_t cmd;

    cmd.super.sig    = EV_COMANDO;
    cmd.para_catraca = para_catraca;
    cmd.sequencia    = sequencia;
    CHECK(ao_post(&sensor.super, &cmd.super, sizeof(cmd)) == AO_OK);
}

static void teste_cadeia_de_threads(void)
{
    const ao_port_cfg_t cfg_catraca = { catraca_fila, 8U };
    const ao_port_cfg_t cfg_sensor  = { sensor_fila, 8U };
    const uint16_t sequencia[] = {
        EV_MOEDA, EV_EMPURRAR, EV_EMPURRAR, EV_FALHA_SENSOR, EV_RESET
    };
    uint32_t i;

    ao_init(&catraca.super, catraca_setup, catraca_handler);
    ao_init(&sensor.super, sensor_setup, sensor_handler);
    CHECK(ao_start(&catraca.super, &cfg_catraca) == AO_OK);
    CHECK(ao_start(&sensor.super, &cfg_sensor) == AO_OK);

    for (i = 0U; i < 5U; i++) {
        posta_comando(sequencia[i], i);
    }

    /* Ordem de parada: primeiro quem produz, depois quem consome -- assim
     * todo repasse do sensor chega a catraca antes do pedido de parada. */
    CHECK(ao_stop(&sensor.super) == AO_OK);
    ao_join(&sensor.super);
    CHECK(ao_stop(&catraca.super) == AO_OK);
    ao_join(&catraca.super);

    CHECK(sensor.fora_de_ordem == 0U);
    CHECK(sensor.proxima_sequencia == 5U);
    CHECK(catraca.despachados == 5U);
    CHECK(fsm_is_in(&catraca.sm, st_travada));

    /* 4 transicoes reais; o segundo EV_EMPURRAR foi ignorado. */
    CHECK(trace_len == 4U);
    CHECK(trace_para[0] == st_liberada);
    CHECK(trace_para[1] == st_travada);
    CHECK(trace_para[2] == st_manutencao);
    CHECK(trace_para[3] == st_travada);
}

static void teste_fila_cheia(void)
{
    const ao_port_cfg_t cfg = { cheio_fila, (size_t)CHEIO_LEN };
    size_t i;

    ao_init(&cheio.super, cheio_setup, cheio_handler);
    CHECK(ao_start(&cheio.super, &cfg) == AO_OK);

    ao_join(&cheio.super); /* a parada vem do proprio AO (cheio_handler) */

    for (i = 0U; i < (size_t)CHEIO_LEN; i++) {
        CHECK(cheio.resultados[i] == AO_OK);
    }
    CHECK(cheio.resultados[CHEIO_LEN] == AO_ERR_QUEUE_FULL);
    CHECK(cheio.despachados == (uint32_t)CHEIO_LEN);
}

int main(void)
{
    teste_cadeia_de_threads();
    teste_fila_cheia();

    (void)printf("test_ao: OK\n");
    return 0;
}
