/* ---------------------------------- ao.h ---------------------------------- */
/**
 * @file ao.h
 * @brief Active Object: uma maquina de estado (fsm ou hsm) com fila de
 *        eventos e contexto de execucao (task/thread) proprios. Eventos
 *        postados de qualquer contexto sao copiados para a fila e
 *        despachados um de cada vez, ate o fim, pela task do objeto.
 * @copyright Bruno Luiz Demarchi (c) 2026
 *
 * A fila e a task vem de uma porta escolhida no build (posix, win32 ou
 * freertos -- ver ao_port.h de cada uma em ports/). fsm e hsm nao sabem que
 * esta camada existe: a maquina e a mesma testada de forma sincrona.
 *
 * Uso, com uma fsm (hsm e igual, trocando fsm_* por hsm_*):
 *
 *     typedef struct {
 *         ao_t  super;                 // sempre o primeiro membro
 *         fsm_t sm;
 *     } catraca_ao_t;
 *
 *     static void setup(ao_t *const me)
 *     { fsm_init(&((catraca_ao_t *)me)->sm, st_travada); }
 *
 *     static void handler(ao_t *const me, const sm_event_t *const e)
 *     { fsm_dispatch(&((catraca_ao_t *)me)->sm, e); }
 *
 *     static catraca_ao_t catraca;
 *     static ao_msg_t     fila[8];
 *
 *     ao_init(&catraca.super, setup, handler);
 *     ao_start(&catraca.super, &cfg);        // cfg: ver ao_port.h da porta
 *     ao_post(&catraca.super, &ev, sizeof ev);
 */

#ifndef AO_H
#define AO_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sm/sm_assert.h"
#include "sm/sm_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Defines -------------------------------------------------------------------*/

/**
 * Maior evento postavel, em bytes (sm_event_t mais payload). Define o
 * tamanho de cada posicao de fila -- toda unidade de traducao precisa ver o
 * mesmo valor, entao defina-o no build (o CMake desta biblioteca propaga a
 * variavel SM_AO_EVENT_SIZE_MAX). Minimo sizeof(sm_event_t).
 */
#ifndef AO_EVENT_SIZE_MAX
#define AO_EVENT_SIZE_MAX 16
#endif

/* Enums ---------------------------------------------------------------------*/

/** Resultado das operacoes que podem falhar em runtime. */
typedef enum {
    AO_OK = 0,         /**< Sucesso. */
    AO_ERR_QUEUE_FULL, /**< Fila cheia: evento NAO postado; quem posta decide. */
    AO_ERR_PORT        /**< A porta nao conseguiu criar fila ou task. */
} ao_err_t;

/* Typedefs ------------------------------------------------------------------*/

typedef struct ao ao_t;

/** Chamada uma vez, na task do objeto, antes do primeiro evento --
 *  tipicamente fsm_init/hsm_init, para que as acoes de entrada rodem no
 *  mesmo contexto que todo o resto da maquina. */
typedef void (*ao_setup_fn)(ao_t *const me);

/** Chamada na task do objeto para cada evento -- tipicamente
 *  fsm_dispatch/hsm_dispatch. `e` so vale durante a chamada. */
typedef void (*ao_handler_fn)(ao_t *const me, const sm_event_t *const e);

/** Armazenamento de um evento na fila; o alinhamento cobre ponteiro e
 *  inteiro de 64 bits no payload. */
typedef union {
    sm_event_t event;
    uint8_t    bytes[AO_EVENT_SIZE_MAX];
    uint64_t   align_u64;
    void      *align_ptr;
} ao_event_storage_t;

/** Posicao de fila. A aplicacao so declara arrays deste tipo para a porta
 *  (campos privados por convencao). */
typedef struct {
    uint8_t            kind;    /**< Evento ou pedido de parada. */
    ao_event_storage_t storage; /**< Copia do evento postado. */
} ao_msg_t;

#ifdef __cplusplus
}
#endif

/* A porta define ao_port_t (fila e task) e ao_port_cfg_t (parametros de
 * ao_start). O diretorio da porta escolhida esta no caminho de include.
 * Fora do extern "C": a porta inclui headers do sistema/RTOS. */
#include "ao_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Atributos da classe. Privados por convencao. */
struct ao {
    ao_setup_fn   setup;   /**< Ver ao_setup_fn; nao nulo. */
    ao_handler_fn handler; /**< Ver ao_handler_fn; nao nulo. */
    bool          started; /**< true a partir de ao_start. */
    ao_port_t     port;    /**< Fila e task, definidos pela porta. */
};

/* Class' public operations (methods) declarations ---------------------------*/

/**
 * @brief  Construtor: associa o objeto as funcoes da maquina. Nao cria fila
 *         nem task e nao executa nada da maquina.
 * @param  me      Objeto; nao pode ser nulo. Deve viver enquanto a task
 *                 existir (estatico, na pratica).
 * @param  setup   Ver ao_setup_fn; nao pode ser nulo.
 * @param  handler Ver ao_handler_fn; nao pode ser nulo.
 */
void ao_init(ao_t *const me, ao_setup_fn setup, ao_handler_fn handler);

/**
 * @brief  Cria a fila e a task do objeto e a coloca para rodar: a task
 *         chama `setup` e depois despacha os eventos na ordem de chegada.
 * @param  me  Objeto ja construido por ao_init; nao pode ser nulo.
 * @param  cfg Parametros da porta (ao_port.h); nao pode ser nulo. A memoria
 *             apontada por ele (fila, pilha) deve viver enquanto a task
 *             existir.
 * @pre    Contexto de tarefa ou inicializacao, nunca interrupcao. Objeto
 *         ainda nao iniciado.
 * @return AO_OK, ou AO_ERR_PORT se a porta falhar (nada fica criado).
 */
ao_err_t ao_start(ao_t *const me, const ao_port_cfg_t *const cfg);

/**
 * @brief  Copia o evento para o fim da fila do objeto. Nao bloqueia: com a
 *         fila cheia o evento e descartado e o erro volta para quem posta.
 * @param  me         Objeto iniciado; nao pode ser nulo.
 * @param  e          Evento; nao nulo, com sinal >= SM_SIG_USER. Pode ser
 *                    descartado logo apos a chamada (e copiado).
 * @param  size_bytes Tamanho do evento (sizeof do tipo derivado), de
 *                    sizeof(sm_event_t) a AO_EVENT_SIZE_MAX.
 * @pre    Contexto de tarefa/thread -- inclusive a do proprio objeto. De
 *         interrupcao, use ao_post_from_isr.
 * @return AO_OK ou AO_ERR_QUEUE_FULL.
 */
ao_err_t ao_post(ao_t *const me, const sm_event_t *const e, size_t size_bytes);

/**
 * @brief  Igual a ao_post, para contexto de interrupcao. Nas portas de host
 *         (posix, win32), que nao tem interrupcao, equivale a ao_post.
 */
ao_err_t ao_post_from_isr(ao_t *const me, const sm_event_t *const e,
                          size_t size_bytes);

/**
 * @brief  Pede a parada da task: um pedido vai para o fim da fila, entao os
 *         eventos postados antes dele ainda sao despachados. Nao espera a
 *         task terminar (nas portas de host, use ao_join para isso).
 * @param  me Objeto iniciado; nao pode ser nulo.
 * @pre    Contexto de tarefa/thread, fora da task do proprio objeto se for
 *         esperar por ela com ao_join.
 * @return AO_OK ou AO_ERR_QUEUE_FULL (pedido nao registrado; tente de novo).
 */
ao_err_t ao_stop(ao_t *const me);

#ifdef __cplusplus
}
#endif

#endif /* AO_H */
