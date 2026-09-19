/* ------------------------------ sm_event.h -------------------------------- */
/**
 * @file sm_event.h
 * @brief Evento e sinais reservados, compartilhados por fsm, hsm e ao.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#ifndef SM_EVENT_H
#define SM_EVENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Sinais reservados, gerados pela propria biblioteca -- nunca pelo mundo
 * externo. Sinais da aplicacao comecam em SM_SIG_USER:
 *
 *     enum { EV_MOEDA = SM_SIG_USER, EV_EMPURRAR, ... };
 */
enum {
    SM_SIG_EMPTY = 0, /**< hsm: pergunta o pai do estado; nunca deve ser tratado. */
    SM_SIG_ENTRY,     /**< fsm/hsm: o estado acabou de ser ativado. */
    SM_SIG_EXIT,      /**< fsm/hsm: o estado esta sendo desativado. */
    SM_SIG_INIT,      /**< hsm: transicao inicial de um estado composto. */
    SM_SIG_USER       /**< Primeiro sinal livre para a aplicacao. */
};

/**
 * Evento minimo: so o sinal. Evento com dado associado e uma struct propria
 * com sm_event_t como PRIMEIRO membro -- o ponteiro para ela converte para
 * `const sm_event_t *` e volta, porque os enderecos coincidem:
 *
 *     typedef struct {
 *         sm_event_t super;       // sempre o primeiro membro
 *         uint16_t   codigo;
 *     } ev_falha_t;
 */
typedef struct {
    uint16_t sig; /**< Sinal; 0 a SM_SIG_USER-1 reservados (acima). */
} sm_event_t;

#ifdef __cplusplus
}
#endif

#endif /* SM_EVENT_H */
