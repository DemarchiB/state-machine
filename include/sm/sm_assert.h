/* ------------------------------ sm_assert.h ------------------------------- */
/**
 * @file sm_assert.h
 * @brief Verificacao de contrato e asserção estatica compartilhadas pelos
 *        componentes da biblioteca (fsm, hsm, ao).
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#ifndef SM_ASSERT_H
#define SM_ASSERT_H

#include <assert.h>

/**
 * Verifica uma pre-condicao de contrato (bug de quem chama, nao erro de
 * runtime -- ver docs/decisions/ADR-0002-fsm-dispatch-void-assert.md).
 *
 * Padrao: assert() de <assert.h>, que some com NDEBUG. Para trocar o
 * comportamento (por exemplo, levar o produto a estado seguro e registrar a
 * falha), defina SM_ASSERT antes de incluir qualquer header da biblioteca --
 * de preferencia como definicao de compilacao do projeto inteiro, para que
 * todas as unidades de traducao usem a mesma versao.
 */
#ifndef SM_ASSERT
#define SM_ASSERT(cond) assert(cond)
#endif

/**
 * Asserção de compilacao em C99 (sem _Static_assert): um typedef de array
 * com tamanho negativo quando a condicao e falsa. `nome` precisa ser unico
 * no escopo.
 */
#define SM_STATIC_ASSERT(cond, nome) \
    typedef char sm_static_assert_##nome[(cond) ? 1 : -1]

#endif /* SM_ASSERT_H */
