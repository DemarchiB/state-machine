/* --------------------------------- fsm.h ---------------------------------- */
/**
 * @file fsm.h
 * @brief Maquina de estados finitos plana (FSM): cada estado e uma funcao,
 *        com acoes de entrada e saida disparadas pela propria biblioteca.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#ifndef FSM_H
#define FSM_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stddef.h>

#include "sm/sm_assert.h"
#include "sm/sm_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Defines -------------------------------------------------------------------*/

/**
 * Trace de transicoes: 1 liga, 0 desliga (padrao). Com 1, fsm_tran chama
 * fsm_trace() a cada transicao, e a aplicacao precisa fornecer o corpo dessa
 * funcao. Como fsm_tran e inline, o valor vale por unidade de traducao --
 * defina-o no build do projeto, nao num arquivo isolado.
 */
#ifndef SM_TRACE
#define SM_TRACE 0
#endif

/* Typedefs ------------------------------------------------------------------*/

typedef struct fsm fsm_t;

/**
 * Funcao de estado: recebe todo evento despachado enquanto o estado esta
 * ativo, inclusive SM_SIG_ENTRY/SM_SIG_EXIT gerados pela biblioteca. Sinal
 * que o estado nao trata e simplesmente ignorado (retorno sem acao).
 */
typedef void (*fsm_state_fn)(fsm_t *const me, const sm_event_t *const e);

/** Atributos da classe. Privados por convencao: a aplicacao consulta o
 *  estado com fsm_is_in e troca de estado so com fsm_tran. */
struct fsm {
    fsm_state_fn state; /**< Estado ativo; nunca nulo apos fsm_init. */
};

/* Class' public operations (methods) declarations ---------------------------*/

/**
 * @brief  Construtor: associa a maquina ao estado inicial e dispara o
 *         SM_SIG_ENTRY dele -- sem isso a acao de entrada do primeiro estado
 *         nunca rodaria, e o mundo fisico poderia nao bater com o que o
 *         software supoe.
 * @param  me            Objeto; nao pode ser nulo.
 * @param  initial_state Estado inicial; nao pode ser nulo.
 * @pre    Primeira chamada sobre o objeto; nenhum fsm_dispatch antes.
 */
void fsm_init(fsm_t *const me, fsm_state_fn initial_state);

/**
 * @brief  Despacha um evento para o estado ativo, ate o fim (run-to-
 *         completion). Sincrono: nao bloqueia, nao enfileira, nao aloca.
 * @param  me Objeto; nao pode ser nulo.
 * @param  e  Evento; nao nulo, com sinal >= SM_SIG_USER (os reservados sao
 *            gerados pela biblioteca). So precisa valer durante a chamada.
 * @pre    fsm_init ja foi chamado. Nao reentrante: nao chame fsm_dispatch
 *         sobre a mesma maquina de dentro de uma funcao de estado, nem de
 *         dois contextos de execucao ao mesmo tempo.
 */
void fsm_dispatch(fsm_t *const me, const sm_event_t *const e);

#if (SM_TRACE == 1)
/**
 * @brief  Hook de trace, chamado por fsm_tran a cada transicao real (evento
 *         ignorado nunca chega aqui). Declarado pela biblioteca e
 *         IMPLEMENTADO pela aplicacao (printf, buffer circular, UART...).
 * @param  source Estado de origem.
 * @param  target Estado de destino.
 */
void fsm_trace(fsm_state_fn source, fsm_state_fn target);
#endif

/**
 * @brief  Indica se a maquina esta no estado informado.
 * @param  me    Objeto; nao pode ser nulo.
 * @param  state Estado a comparar.
 * @return true se `state` e o estado ativo.
 */
static inline bool fsm_is_in(const fsm_t *const me, fsm_state_fn state)
{
    SM_ASSERT(me != NULL);
    return me->state == state;
}

/**
 * @brief  Transiciona para outro estado: SM_SIG_EXIT no estado ativo, troca,
 *         SM_SIG_ENTRY no novo. Unico ponto de troca de estado.
 * @param  me     Objeto; nao pode ser nulo.
 * @param  target Estado de destino; nao pode ser nulo. Pode ser o proprio
 *                estado ativo (auto-transicao: sai e entra de novo).
 * @pre    Chamada de dentro de uma funcao de estado, durante fsm_dispatch.
 *         Nao chame fsm_tran de dentro de SM_SIG_ENTRY/SM_SIG_EXIT.
 *
 * static inline porque e chamada pelas funcoes de estado da aplicacao a cada
 * transicao e o corpo e trivial.
 */
static inline void fsm_tran(fsm_t *const me, fsm_state_fn target)
{
    static const sm_event_t exit_ev  = { SM_SIG_EXIT };
    static const sm_event_t entry_ev = { SM_SIG_ENTRY };
#if (SM_TRACE == 1)
    fsm_state_fn source;
#endif

    SM_ASSERT(me != NULL);
    SM_ASSERT(target != NULL);

#if (SM_TRACE == 1)
    source = me->state;
#endif
    me->state(me, &exit_ev);   /* EXIT ainda ve o estado antigo como ativo */
    me->state = target;
    me->state(me, &entry_ev);

#if (SM_TRACE == 1)
    fsm_trace(source, target);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* FSM_H */
