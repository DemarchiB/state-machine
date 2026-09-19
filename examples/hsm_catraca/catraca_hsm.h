/* ----------------------------- catraca_hsm.h ------------------------------ */
/**
 * @file catraca_hsm.h
 * @brief Exemplo de hsm: catraca com tres causas de falha independentes,
 *        resolvidas por um estado pai em vez de regras repetidas.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#ifndef CATRACA_HSM_H
#define CATRACA_HSM_H

#include <stdbool.h>
#include <stdint.h>

#include "sm/hsm.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Sinais da aplicacao: comecam em SM_SIG_USER. */
enum {
    EV_MOEDA = SM_SIG_USER,   /**< Moeda (ou credito) aceita. */
    EV_EMPURRAR,              /**< Alguem empurrou o braco da catraca. */
    EV_FALHA_SENSOR,          /**< Falha do sensor de passagem. */
    EV_FALHA_ATUADOR,         /**< Falha do atuador da trava. */
    EV_FALHA_COMUNICACAO,     /**< Falha de comunicacao com o leitor. */
    EV_RESET                  /**< Reset manual pelo tecnico. */
};

/** A maquina, com hsm_t como primeiro membro, e os dados dela. */
typedef struct {
    hsm_t    super;          /**< Sempre o primeiro membro. */
    bool     trava_fechada;  /**< Saida fisica simulada: true = fechada. */
    uint16_t falhas;         /**< Falhas registradas desde o construtor. */
} catraca_hsm_t;

/** @brief Construtor: zera os dados e entra na hierarquia (hsm_init). */
void catraca_hsm_init(catraca_hsm_t *const me);

hsm_status_t st_catraca(hsm_t *const me, const sm_event_t *const e);
hsm_status_t st_operacional(hsm_t *const me, const sm_event_t *const e);
hsm_status_t st_travada(hsm_t *const me, const sm_event_t *const e);
hsm_status_t st_liberada(hsm_t *const me, const sm_event_t *const e);
hsm_status_t st_falha_segura(hsm_t *const me, const sm_event_t *const e);
hsm_status_t st_falha_sensor(hsm_t *const me, const sm_event_t *const e);
hsm_status_t st_falha_atuador(hsm_t *const me, const sm_event_t *const e);
hsm_status_t st_falha_comunicacao(hsm_t *const me, const sm_event_t *const e);

#ifdef __cplusplus
}
#endif

#endif /* CATRACA_HSM_H */
