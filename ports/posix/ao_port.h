/* ------------------------------- ao_port.h -------------------------------- */
/**
 * @file ao_port.h
 * @brief Porta POSIX (host Linux/macOS) do Active Object: thread pthread e
 *        fila circular protegida por mutex + variavel de condicao. Incluido
 *        por sm/ao.h -- nao inclua diretamente.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#ifndef AO_PORT_H
#define AO_PORT_H

#include <pthread.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Parametros de ao_start nesta porta. */
typedef struct {
    ao_msg_t *queue_storage; /**< Posicoes da fila, da aplicacao; nao nulo. */
    size_t    queue_len;     /**< Numero de posicoes em queue_storage, >= 1. */
} ao_port_cfg_t;

/** Estado da porta dentro de ao_t. Privado. */
typedef struct {
    pthread_t       thread;
    pthread_mutex_t lock;
    pthread_cond_t  not_empty;
    ao_msg_t       *buf;
    size_t          len;
    size_t          head;  /**< Proxima posicao a ler. */
    size_t          count; /**< Posicoes ocupadas, 0 a len. */
} ao_port_t;

/**
 * @brief  Espera a thread do objeto terminar (depois de ao_stop) e libera
 *         as primitivas da porta. O objeto pode ser iniciado de novo.
 * @param  me Objeto iniciado; nao pode ser nulo.
 * @pre    Fora da thread do proprio objeto. So existe nas portas de host.
 */
void ao_join(ao_t *const me);

#ifdef __cplusplus
}
#endif

#endif /* AO_PORT_H */
