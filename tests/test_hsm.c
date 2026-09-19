/* ------------------------------- test_hsm.c ------------------------------- */
/**
 * @file test_hsm.c
 * @brief Teste em host do algoritmo da hsm, com uma hierarquia sintetica
 *        que cobre cada forma de transicao. Toda acao de entrada, saida e
 *        transicao inicial e registrada num log, e o teste confere a
 *        sequencia exata -- a ordem e o que a hierarquia garante.
 * @copyright Bruno Luiz Demarchi (c) 2026
 *
 *   hsm_top
 *   └── s              inicial -> s11 (desce dois niveis)   C -> s1
 *       ├── s1         inicial -> s11   A -> s1 (auto)      F: tratado, sem transicao
 *       │   └── s11                     B -> s211 (primo)   D -> s (ancestral)
 *       └── s2         inicial -> s21
 *           └── s21    inicial -> s211  G -> s2 (ancestral do tratador)
 *               └── s211
 */

#include "sm/hsm.h"

#include <stdio.h>
#include <string.h>

#include "check.h"

enum { SIG_A = SM_SIG_USER, SIG_B, SIG_C, SIG_D, SIG_E, SIG_F, SIG_G };

/* --- log de acoes ---------------------------------------------------------*/

static char   log_buf[512];
static size_t trace_count;

static void log_clear(void)
{
    log_buf[0] = '\0';
}

/* memcpy em vez de strcat: strcat e depreciado no MSVC (C4996). */
static void log_add(const char *texto)
{
    size_t usado = strlen(log_buf);
    const size_t n = strlen(texto);

    CHECK(usado + n + 2U < sizeof(log_buf));
    if (usado > 0U) {
        log_buf[usado] = ' ';
        usado++;
    }
    (void)memcpy(&log_buf[usado], texto, n + 1U); /* inclui o '\0' */
}

static void check_log(const char *esperado, int linha)
{
    if (strcmp(log_buf, esperado) != 0) {
        (void)fprintf(stderr, "linha %d:\n  esperado: [%s]\n  obtido:   [%s]\n",
                      linha, esperado, log_buf);
        exit(EXIT_FAILURE);
    }
}
#define CHECK_LOG(esperado) check_log((esperado), __LINE__)

void hsm_trace(hsm_state_fn source, hsm_state_fn target)
{
    (void)source;
    (void)target;
    trace_count++;
}

/* --- estados ----------------------------------------------------------------
 * Cada estado registra ENTRY/EXIT como "<nome>:E"/"<nome>:X" e a transicao
 * inicial como "<nome>:I". */

static hsm_status_t s(hsm_t *const me, const sm_event_t *const e);
static hsm_status_t s1(hsm_t *const me, const sm_event_t *const e);
static hsm_status_t s11(hsm_t *const me, const sm_event_t *const e);
static hsm_status_t s2(hsm_t *const me, const sm_event_t *const e);
static hsm_status_t s21(hsm_t *const me, const sm_event_t *const e);
static hsm_status_t s211(hsm_t *const me, const sm_event_t *const e);

/* Trata ENTRY/EXIT de forma igual para todos; devolve true se tratou. */
static bool entry_exit(const char *nome, const sm_event_t *const e)
{
    char texto[16];

    if (e->sig == SM_SIG_ENTRY) {
        (void)snprintf(texto, sizeof(texto), "%s:E", nome);
        log_add(texto);
        return true;
    }
    if (e->sig == SM_SIG_EXIT) {
        (void)snprintf(texto, sizeof(texto), "%s:X", nome);
        log_add(texto);
        return true;
    }
    return false;
}

static hsm_status_t s(hsm_t *const me, const sm_event_t *const e)
{
    if (entry_exit("s", e)) {
        return HSM_HANDLED;
    }
    switch (e->sig) {
    case SM_SIG_INIT:
        log_add("s:I");
        return hsm_tran(me, s11);
    case SIG_C:
        return hsm_tran(me, s1);
    default:
        return hsm_super(me, hsm_top);
    }
}

static hsm_status_t s1(hsm_t *const me, const sm_event_t *const e)
{
    if (entry_exit("s1", e)) {
        return HSM_HANDLED;
    }
    switch (e->sig) {
    case SM_SIG_INIT:
        log_add("s1:I");
        return hsm_tran(me, s11);
    case SIG_A:
        return hsm_tran(me, s1);
    case SIG_F:
        log_add("s1:F");
        return HSM_HANDLED;
    default:
        return hsm_super(me, s);
    }
}

static hsm_status_t s11(hsm_t *const me, const sm_event_t *const e)
{
    if (entry_exit("s11", e)) {
        return HSM_HANDLED;
    }
    switch (e->sig) {
    case SIG_B:
        return hsm_tran(me, s211);
    case SIG_D:
        return hsm_tran(me, s);
    default:
        return hsm_super(me, s1);
    }
}

static hsm_status_t s2(hsm_t *const me, const sm_event_t *const e)
{
    if (entry_exit("s2", e)) {
        return HSM_HANDLED;
    }
    switch (e->sig) {
    case SM_SIG_INIT:
        log_add("s2:I");
        return hsm_tran(me, s21);
    default:
        return hsm_super(me, s);
    }
}

static hsm_status_t s21(hsm_t *const me, const sm_event_t *const e)
{
    if (entry_exit("s21", e)) {
        return HSM_HANDLED;
    }
    switch (e->sig) {
    case SM_SIG_INIT:
        log_add("s21:I");
        return hsm_tran(me, s211);
    case SIG_G:
        return hsm_tran(me, s2);
    default:
        return hsm_super(me, s2);
    }
}

static hsm_status_t s211(hsm_t *const me, const sm_event_t *const e)
{
    if (entry_exit("s211", e)) {
        return HSM_HANDLED;
    }
    return hsm_super(me, s21);
}

static void envia(hsm_t *const me, uint16_t sig)
{
    sm_event_t ev;

    ev.sig = sig;

    log_clear();
    hsm_dispatch(me, &ev);
}

int main(void)
{
    hsm_t m;
    size_t traces;

    /* Construtor: entra de cima para baixo e segue a inicial de s, que
     * desce dois niveis de uma vez. */
    log_clear();
    hsm_init(&m, s);
    CHECK_LOG("s:E s:I s1:E s11:E");
    CHECK(hsm_state(&m) == s11);
    CHECK(trace_count == 1U);

    /* hsm_is_in: a folha e todos os ancestrais estao ativos. */
    CHECK(hsm_is_in(&m, s11));
    CHECK(hsm_is_in(&m, s1));
    CHECK(hsm_is_in(&m, s));
    CHECK(hsm_is_in(&m, hsm_top));
    CHECK(!hsm_is_in(&m, s2));
    CHECK(!hsm_is_in(&m, s211));

    /* Tratado pelo pai sem transicao: nenhuma entrada/saida. */
    traces = trace_count;
    envia(&m, SIG_F);
    CHECK_LOG("s1:F");
    CHECK(hsm_state(&m) == s11);
    CHECK(trace_count == traces);

    /* Ninguem trata: sobe ate hsm_top e e ignorado. */
    envia(&m, SIG_E);
    CHECK_LOG("");
    CHECK(hsm_state(&m) == s11);
    CHECK(trace_count == traces);

    /* Auto-transicao de s1 (tratada no pai da folha): sai e entra em s1. */
    envia(&m, SIG_A);
    CHECK_LOG("s11:X s1:X s1:E s1:I s11:E");
    CHECK(hsm_state(&m) == s11);

    /* Primos: LCA = s, que nao sai nem entra; iniciais de s2/s21 nao rodam
     * porque o alvo ja e a folha s211. */
    envia(&m, SIG_B);
    CHECK_LOG("s11:X s1:X s2:E s21:E s211:E");
    CHECK(hsm_state(&m) == s211);

    /* Tratado por s21 com alvo s2, ancestral de quem tratou: s2 sai e entra
     * de novo, e as iniciais de s2 e s21 descem ate a folha. */
    envia(&m, SIG_G);
    CHECK_LOG("s211:X s21:X s2:X s2:E s2:I s21:E s21:I s211:E");
    CHECK(hsm_state(&m) == s211);

    /* Tratado por s com alvo s1, descendente de s: transicao local, s nao
     * sai; os estados abaixo de s saem. */
    envia(&m, SIG_C);
    CHECK_LOG("s211:X s21:X s2:X s1:E s1:I s11:E");
    CHECK(hsm_state(&m) == s11);

    /* Alvo e ancestral da folha: LCA = hsm_top, s sai e entra. */
    envia(&m, SIG_D);
    CHECK_LOG("s11:X s1:X s:X s:E s:I s1:E s11:E");
    CHECK(hsm_state(&m) == s11);

    (void)printf("test_hsm: OK\n");
    return 0;
}
