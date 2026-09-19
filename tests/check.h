/* -------------------------------- check.h --------------------------------- */
/**
 * @file check.h
 * @brief Verificacao minima para os testes em host: ao contrario de
 *        assert(), continua valendo com NDEBUG (build Release), para que um
 *        teste nunca passe por ter sido compilado sem as verificacoes.
 * @copyright Bruno Luiz Demarchi (c) 2026
 */

#ifndef CHECK_H
#define CHECK_H

#include <stdio.h>
#include <stdlib.h>

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (!(cond)) {                                                     \
            (void)fprintf(stderr, "%s:%d: falhou: %s\n", __FILE__,         \
                          __LINE__, #cond);                                \
            exit(EXIT_FAILURE);                                            \
        }                                                                  \
    } while (0)

#endif /* CHECK_H */
