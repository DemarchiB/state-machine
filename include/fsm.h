/* --------------------------------- fsm.h ---------------------------------- */
/**
 * @file fsm.h
 * @brief Interface publica da biblioteca de maquinas de estado finitas (FSM).
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#ifndef FSM_H
#define FSM_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Enums -----------------------------------------------------------------------
 * So a interface publica esta decidida por enquanto: fsm_t, event_t,
 * fsm_dispatch. Sinais reservados (SIG_ENTRY/SIG_EXIT) e uma funcao de
 * transicao de conveniencia entram junto com a primeira implementacao real.
 * ----------------------------------------------------------------------------*/

/** Codigo de retorno das operacoes da biblioteca. */
typedef enum {
    FSM_OK = 0,       /**< Operacao concluida sem erro. */
    FSM_ERR_PARAM     /**< Algum parametro obrigatorio era invalido (ex.: NULL). */
} fsm_err_t;

/* Typedefs --------------------------------------------------------------------*/

typedef struct fsm fsm_t;

/**
 * Evento minimo: so o sinal. Uma maquina que precisar de payload define seu
 * proprio tipo com event_t como PRIMEIRO membro (mesmo idioma de composicao
 * de docs/guide/templates/modulo-c.md, Secao "Polimorfismo, quando ele se
 * justifica") -- nao existe payload generico aqui ainda (ver
 * ARCHITECTURE.md, "Pontos nao determinados").
 */
typedef struct {
    uint16_t sig;   /**< Identificador do evento/sinal. */
} event_t;

/** Funcao de estado: cada estado da maquina e uma funcao com esta assinatura. */
typedef void (*fsm_state_fn)(fsm_t *me, const event_t *e);

/** Atributos da classe. Campo privado por convencao: so as operacoes deste
 *  modulo devem tocar em `estado` diretamente. */
struct fsm {
    fsm_state_fn estado;
};

/* Class' public operations (methods) declarations ---------------------------*/

/**
 * @brief  Construtor: associa a maquina ao seu estado inicial.
 * @param  me             Ponteiro para o objeto; nao pode ser nulo.
 * @param  estado_inicial Funcao do primeiro estado da maquina; nao pode ser nula.
 * @return FSM_OK, ou FSM_ERR_PARAM se algum argumento for invalido.
 */
fsm_err_t fsm_init(fsm_t *const me, fsm_state_fn estado_inicial);

/**
 * @brief  Despacha um evento para o estado atual da maquina. Sincrono: nao
 *         bloqueia, nao enfileira (ver ADR-0001 em docs/decisions/).
 * @param  me Ponteiro para o objeto; nao pode ser nulo.
 * @param  e  Evento a despachar; nao pode ser nulo.
 * @return FSM_OK, ou FSM_ERR_PARAM se algum argumento for invalido.
 */
fsm_err_t fsm_dispatch(fsm_t *const me, const event_t *const e);

#ifdef __cplusplus
}
#endif

#endif /* FSM_H */
