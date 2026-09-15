/* --------------------------------- fsm.h ---------------------------------- */
/**
 * @file fsm.h
 * @brief Interface publica da biblioteca de maquinas de estado finitas (FSM).
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#ifndef FSM_H
#define FSM_H

/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Defines ---------------------------------------------------------------------
 * Contrato: ponteiro nulo em qualquer parametro e bug de quem chama, nao
 * erro de runtime -- por isso assert, nao retorno de erro. Ver
 * docs/decisions/ADR-0002-fsm-dispatch-void-assert.md.
 * ----------------------------------------------------------------------------*/
#ifndef FSM_ASSERT
#define FSM_ASSERT(cond) assert(cond)
#endif

/* Typedefs --------------------------------------------------------------------*/

typedef struct fsm fsm_t;

/**
 * Evento minimo: so o sinal. Uma maquina que precisar de payload define seu
 * proprio tipo com event_t como PRIMEIRO membro (mesmo idioma de composicao
 * de docs/guide/templates/modulo-c.md, Secao "Polimorfismo, quando ele se
 * justifica").
 */
typedef struct {
    uint16_t sig;   /**< Identificador do evento/sinal. */
} event_t;

/** Funcao de estado: cada estado da maquina e uma funcao com esta assinatura. */
typedef void (*fsm_state_fn)(fsm_t *me, const event_t *e);

/** Atributos da classe. Campo privado por convencao: so as operacoes deste
 *  modulo devem tocar em `state` diretamente -- funcoes de estado da
 *  aplicacao trocam de estado via fsm_tran, nunca por atribuicao direta. */
struct fsm {
    fsm_state_fn state;
};

/* Enums -------------------------------------------------------------------
 * Sinais reservados, gerados pela propria biblioteca (nunca pelo mundo
 * externo) na troca de estado. Sinais da aplicacao comecam em SIG_USER.
 * ----------------------------------------------------------------------------*/
enum {
    SIG_ENTRY = 0,
    SIG_EXIT,
    SIG_USER
};

/* Class' public operations (methods) declarations ---------------------------*/

/**
 * @brief  Construtor: associa a maquina ao seu estado inicial e dispara o
 *         SIG_ENTRY desse estado -- sem isso a acao de entrada do primeiro
 *         estado nunca roda (o mundo fisico pode nao bater com o que o
 *         software supoe).
 * @param  me            Ponteiro para o objeto; nao pode ser nulo.
 * @param  initial_state Funcao do primeiro estado da maquina; nao pode ser nula.
 * @pre    Nenhum (primeira chamada sobre o objeto).
 */
void fsm_init(fsm_t *const me, fsm_state_fn initial_state);

/**
 * @brief  Despacha um evento para o estado atual da maquina. Sincrono: nao
 *         bloqueia, nao enfileira (ver ADR-0001 em docs/decisions/).
 * @param  me Ponteiro para o objeto; nao pode ser nulo.
 * @param  e  Evento a despachar; nao pode ser nulo.
 * @pre    fsm_init ja foi chamado sobre este objeto.
 */
void fsm_dispatch(fsm_t *const me, const event_t *const e);

/**
 * @brief  Transiciona a maquina para um novo estado: dispara SIG_EXIT no
 *         estado atual, troca me->state, dispara SIG_ENTRY no novo. Ponto
 *         unico de troca de estado -- funcoes de estado da aplicacao nunca
 *         atribuem me->state diretamente, sempre chamam fsm_tran.
 * @param  me          Ponteiro para o objeto; nao pode ser nulo.
 * @param  novo_estado Funcao do estado de destino; nao pode ser nula.
 *
 * Decisao: static inline aqui no header, nao funcao exportada em fsm.c --
 * porque quem chama fsm_tran sao as funcoes de estado da APLICACAO
 * (catraca.c, por exemplo), em todo evento que causa transicao. E o
 * caminho mais frequente da biblioteca depois de fsm_dispatch, e o corpo e
 * trivial (tres chamadas), sem nada que precise ficar escondido no .c.
 */
static inline void fsm_tran(fsm_t *const me, fsm_state_fn novo_estado)
{
    static const event_t EXIT_EV  = { .sig = SIG_EXIT };
    static const event_t ENTRY_EV = { .sig = SIG_ENTRY };

    FSM_ASSERT(me != NULL);
    FSM_ASSERT(novo_estado != NULL);

    me->state(me, &EXIT_EV);    /* avisa o estado atual que ele esta saindo */
    me->state = novo_estado;    /* so agora troca -- EXIT ainda viu o estado antigo */
    me->state(me, &ENTRY_EV);   /* avisa o novo estado que ele acabou de entrar */
}

#ifdef __cplusplus
}
#endif

#endif /* FSM_H */
