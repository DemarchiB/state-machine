/* ------------------------------ ao_port_api.h ----------------------------- */
/**
 * @file ao_port_api.h
 * @brief Contrato interno entre ao.c e as portas (ports/<porta>/ao_port.c).
 *        Nao faz parte da interface publica.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#ifndef AO_PORT_API_H
#define AO_PORT_API_H

#include "sm/ao.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Cria fila e task; a task chama ao_run(me) e, quando ele retornar,
 *         encerra a si mesma.
 * @return AO_OK ou AO_ERR_PORT (nada fica criado).
 */
ao_err_t ao_port_start(ao_t *const me, const ao_port_cfg_t *const cfg);

/** @brief Copia `msg` para o fim da fila sem bloquear. Contexto de tarefa. */
ao_err_t ao_port_post(ao_t *const me, const ao_msg_t *const msg);

/** @brief Igual a ao_port_post, em contexto de interrupcao. */
ao_err_t ao_port_post_from_isr(ao_t *const me, const ao_msg_t *const msg);

/**
 * @brief  Bloqueia a task do objeto ate haver mensagem e a copia para `msg`.
 *         Espera sem tempo limite por projeto: a task de um Active Object so
 *         tem trabalho quando chega evento, e a parada tambem chega pela fila.
 */
void ao_port_receive(ao_t *const me, ao_msg_t *const msg);

/** @brief Laco do objeto (ao.c), chamado pela task criada em ao_port_start. */
void ao_run(ao_t *const me);

#ifdef __cplusplus
}
#endif

#endif /* AO_PORT_API_H */
