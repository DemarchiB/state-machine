/* --------------------------------- hsm.h ---------------------------------- */
/**
 * @file hsm.h
 * @brief Maquina de estados hierarquica (HSM): estados aninhados, com o
 *        evento nao tratado subindo para o estado pai, acoes de entrada e
 *        saida propagadas pela hierarquia e transicao inicial de estado
 *        composto.
 * @copyright Bruno Luiz Demarchi (c) 2026
 *
 * Forma de uma funcao de estado:
 *
 *     static hsm_status_t st_filho(hsm_t *const me, const sm_event_t *const e)
 *     {
 *         switch (e->sig) {
 *         case SM_SIG_ENTRY: liga_saida();       return HSM_HANDLED;
 *         case EV_X:         return hsm_tran(me, st_outro);
 *         default:           return hsm_super(me, st_pai);  // obrigatorio
 *         }
 *     }
 *
 * - O `default` sempre devolve o pai com hsm_super -- e por ele que a
 *   biblioteca descobre a hierarquia. Estado de primeiro nivel devolve
 *   hsm_super(me, hsm_top).
 * - Estado composto trata SM_SIG_INIT com hsm_tran para um filho direto ou
 *   indireto (transicao inicial). Estado folha nao trata SM_SIG_INIT.
 * - Evento ignorado de proposito, num estado que nao quer repassa-lo ao pai,
 *   retorna HSM_HANDLED sem acao.
 */

#ifndef HSM_H
#define HSM_H

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
 * Profundidade maxima da hierarquia, contando hsm_top (um estado de primeiro
 * nivel tem profundidade 2). Dimensiona um array em pilha em hsm_dispatch;
 * excede-la e violacao de contrato (SM_ASSERT).
 */
#ifndef HSM_MAX_DEPTH
#define HSM_MAX_DEPTH 8
#endif

/** Trace de transicoes: 1 liga, 0 desliga (padrao). Ver hsm_trace. */
#ifndef SM_TRACE
#define SM_TRACE 0
#endif

/* Typedefs ------------------------------------------------------------------*/

typedef struct hsm hsm_t;

/** Resultado de uma funcao de estado. Use os helpers hsm_tran/hsm_super. */
typedef enum {
    HSM_HANDLED = 0, /**< Evento tratado (ou ignorado de proposito) aqui. */
    HSM_IGNORED,     /**< Chegou a hsm_top sem ninguem tratar. */
    HSM_SUPER,       /**< Nao tratado: tente o pai (em me->temp). */
    HSM_TRAN         /**< Tratado com transicao para o alvo (em me->temp). */
} hsm_status_t;

/** Funcao de estado hierarquico. */
typedef hsm_status_t (*hsm_state_fn)(hsm_t *const me, const sm_event_t *const e);

/** Atributos da classe. Privados por convencao: a aplicacao consulta o
 *  estado com hsm_is_in/hsm_state e so escreve via hsm_tran/hsm_super. */
struct hsm {
    hsm_state_fn state; /**< Estado ativo -- sempre a folha, nunca um pai. */
    hsm_state_fn temp;  /**< Rascunho: pai (HSM_SUPER) ou alvo (HSM_TRAN). */
};

/* Class' public operations (methods) declarations ---------------------------*/

/**
 * @brief  Raiz implicita da hierarquia: pai de todo estado de primeiro
 *         nivel. Ignora qualquer evento; nunca recebe entrada nem saida.
 */
hsm_status_t hsm_top(hsm_t *const me, const sm_event_t *const e);

/**
 * @brief  Construtor: entra no estado inicial, disparando SM_SIG_ENTRY de
 *         cada ancestral de cima para baixo, e segue as transicoes iniciais
 *         (SM_SIG_INIT) ate uma folha.
 * @param  me            Objeto; nao pode ser nulo.
 * @param  initial_state Estado inicial, folha ou composto; nao nulo e
 *                       diferente de hsm_top.
 * @pre    Primeira chamada sobre o objeto.
 */
void hsm_init(hsm_t *const me, hsm_state_fn initial_state);

/**
 * @brief  Despacha um evento: comeca na folha ativa e sobe pelos pais ate um
 *         estado tratar. Se o tratamento for uma transicao, sai dos estados
 *         ate o ancestral comum (LCA) com a origem, entra ate o alvo e segue
 *         as transicoes iniciais. Sincrono, sem alocacao, sem recursao.
 * @param  me Objeto; nao pode ser nulo.
 * @param  e  Evento; nao nulo, com sinal >= SM_SIG_USER (os reservados sao
 *            gerados pela biblioteca). So precisa valer durante a chamada.
 * @pre    hsm_init ja foi chamado. Nao reentrante (mesmas regras de
 *         fsm_dispatch).
 */
void hsm_dispatch(hsm_t *const me, const sm_event_t *const e);

/**
 * @brief  Indica se `state` esta ativo: e a folha ou um de seus ancestrais.
 * @param  me    Objeto; nao pode ser nulo. Usa me->temp como rascunho.
 * @param  state Estado a consultar.
 * @return true se `state` esta ativo (hsm_top sempre esta).
 * @pre    Fora de hsm_dispatch, no mesmo contexto que despacha os eventos.
 */
bool hsm_is_in(hsm_t *const me, hsm_state_fn state);

#if (SM_TRACE == 1)
/**
 * @brief  Hook de trace, chamado a cada transicao real (inclusive a inicial
 *         de hsm_init, com source == hsm_top). IMPLEMENTADO pela aplicacao.
 * @param  source Folha ativa antes da transicao.
 * @param  target Folha ativa depois da transicao (e das transicoes iniciais).
 */
void hsm_trace(hsm_state_fn source, hsm_state_fn target);
#endif

/**
 * @brief  Estado folha ativo.
 * @param  me Objeto; nao pode ser nulo.
 */
static inline hsm_state_fn hsm_state(const hsm_t *const me)
{
    SM_ASSERT(me != NULL);
    return me->state;
}

/**
 * @brief  Retorno de funcao de estado: "tratei com transicao para target".
 *         A transicao em si e executada por hsm_dispatch/hsm_init, depois
 *         que a funcao de estado retorna.
 * @param  me     Objeto; nao pode ser nulo.
 * @param  target Estado de destino; nao nulo e diferente de hsm_top.
 */
static inline hsm_status_t hsm_tran(hsm_t *const me, hsm_state_fn target)
{
    SM_ASSERT(me != NULL);
    me->temp = target;
    return HSM_TRAN;
}

/**
 * @brief  Retorno de funcao de estado: "nao tratei; meu pai e parent".
 * @param  me     Objeto; nao pode ser nulo.
 * @param  parent Estado pai (hsm_top para estado de primeiro nivel).
 */
static inline hsm_status_t hsm_super(hsm_t *const me, hsm_state_fn parent)
{
    SM_ASSERT(me != NULL);
    me->temp = parent;
    return HSM_SUPER;
}

#ifdef __cplusplus
}
#endif

#endif /* HSM_H */
