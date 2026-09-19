/* --------------------------------- hsm.c ---------------------------------- */
/**
 * @file hsm.c
 * @brief Maquina de estados hierarquica (HSM).
 * @copyright Bruno Luiz Demarchi (c) 2026
 *
 * A hierarquia nao fica numa tabela: cada funcao de estado conhece so o
 * proprio pai (o `default: return hsm_super(me, pai)`), e a biblioteca o
 * descobre enviando SM_SIG_EMPTY, sinal que nenhum estado trata. Caminhos
 * ate a raiz sao montados num array de HSM_MAX_DEPTH posicoes em pilha --
 * sem alocacao e sem recursao.
 */

#include "sm/hsm.h"

/* Private defines -----------------------------------------------------------*/

/** Indice "nao encontrado" em path_find. */
#define PATH_NOT_FOUND ((size_t)0)

/* Private variables ---------------------------------------------------------*/

static const sm_event_t empty_ev = { SM_SIG_EMPTY };
static const sm_event_t entry_ev = { SM_SIG_ENTRY };
static const sm_event_t exit_ev  = { SM_SIG_EXIT };
static const sm_event_t init_ev  = { SM_SIG_INIT };

/* Class' private operations (methods) declaration ---------------------------*/

static hsm_state_fn parent_of(hsm_t *const me, hsm_state_fn state);
static void enter(hsm_t *const me, hsm_state_fn state);
static void leave(hsm_t *const me, hsm_state_fn state);
static size_t path_to_top(hsm_t *const me, hsm_state_fn state,
                          hsm_state_fn path[HSM_MAX_DEPTH]);
static size_t path_find(const hsm_state_fn path[HSM_MAX_DEPTH], size_t len,
                        hsm_state_fn state);
static void enter_down(hsm_t *const me, const hsm_state_fn path[HSM_MAX_DEPTH],
                       size_t from);
static void follow_initial(hsm_t *const me);
static void transition(hsm_t *const me, hsm_state_fn source,
                       hsm_state_fn target);

/* Class' public operations (methods) definition -----------------------------*/

hsm_status_t hsm_top(hsm_t *const me, const sm_event_t *const e)
{
    (void)me;
    (void)e;
    return HSM_IGNORED;
}

void hsm_init(hsm_t *const me, hsm_state_fn initial_state)
{
    hsm_state_fn path[HSM_MAX_DEPTH];
    size_t len;

    SM_ASSERT(me != NULL);
    SM_ASSERT(initial_state != NULL);
    SM_ASSERT(initial_state != hsm_top);

    me->state = hsm_top;
    me->temp  = NULL;

    len = path_to_top(me, initial_state, path);
    enter_down(me, path, len - 1U);   /* path[len-1] e hsm_top */
    me->state = initial_state;
    follow_initial(me);

#if (SM_TRACE == 1)
    hsm_trace(hsm_top, me->state);
#endif
}

void hsm_dispatch(hsm_t *const me, const sm_event_t *const e)
{
    hsm_state_fn handler;
    hsm_status_t status;

    SM_ASSERT(me != NULL);
    SM_ASSERT(me->state != NULL);
    SM_ASSERT(e != NULL);
    SM_ASSERT(e->sig >= (uint16_t)SM_SIG_USER); /* reservados sao internos */

    /* Sobe da folha ate alguem tratar; hsm_top sempre encerra o laco. */
    handler = me->state;
    status  = handler(me, e);
    while (status == HSM_SUPER) {
        SM_ASSERT(me->temp != NULL);
        handler = me->temp;
        status  = handler(me, e);
    }

    if (status == HSM_TRAN) {
        transition(me, handler, me->temp);
    }
}

bool hsm_is_in(hsm_t *const me, hsm_state_fn state)
{
    hsm_state_fn s;

    SM_ASSERT(me != NULL);
    SM_ASSERT(me->state != NULL);

    for (s = me->state; s != hsm_top; s = parent_of(me, s)) {
        if (s == state) {
            return true;
        }
    }
    return state == hsm_top;
}

/* Class' private operations (methods) definition ----------------------------*/

static hsm_state_fn parent_of(hsm_t *const me, hsm_state_fn state)
{
    hsm_status_t status;

    SM_ASSERT(state != hsm_top);
    status = state(me, &empty_ev);
    /* Falha aqui: a funcao de estado nao devolve hsm_super no default. */
    SM_ASSERT(status == HSM_SUPER);
    SM_ASSERT(me->temp != NULL);
    (void)status;
    return me->temp;
}

static void enter(hsm_t *const me, hsm_state_fn state)
{
    hsm_status_t status = state(me, &entry_ev);
    SM_ASSERT(status != HSM_TRAN); /* transicao em SM_SIG_ENTRY e proibida */
    (void)status;
}

static void leave(hsm_t *const me, hsm_state_fn state)
{
    hsm_status_t status = state(me, &exit_ev);
    SM_ASSERT(status != HSM_TRAN); /* transicao em SM_SIG_EXIT e proibida */
    (void)status;
}

/* path[0] = state, path[1] = pai, ..., path[len-1] = hsm_top. */
static size_t path_to_top(hsm_t *const me, hsm_state_fn state,
                          hsm_state_fn path[HSM_MAX_DEPTH])
{
    size_t len = 0U;

    path[len] = state;
    len++;
    while (state != hsm_top) {
        SM_ASSERT(len < (size_t)HSM_MAX_DEPTH); /* aumente HSM_MAX_DEPTH */
        state = parent_of(me, state);
        path[len] = state;
        len++;
    }
    return len;
}

/* Procura `state` entre os ancestrais proprios do alvo (indices 1..len-1). */
static size_t path_find(const hsm_state_fn path[HSM_MAX_DEPTH], size_t len,
                        hsm_state_fn state)
{
    size_t i;

    for (i = 1U; i < len; i++) {
        if (path[i] == state) {
            return i;
        }
    }
    return PATH_NOT_FOUND;
}

/* Entra de path[from-1] ate path[0]: do mais generico ao mais especifico. */
static void enter_down(hsm_t *const me, const hsm_state_fn path[HSM_MAX_DEPTH],
                       size_t from)
{
    while (from > 0U) {
        from--;
        enter(me, path[from]);
    }
}

/* Segue SM_SIG_INIT enquanto o estado ativo for composto. */
static void follow_initial(hsm_t *const me)
{
    hsm_state_fn path[HSM_MAX_DEPTH];
    hsm_state_fn target;
    size_t len;
    size_t idx;

    while (me->state(me, &init_ev) == HSM_TRAN) {
        target = me->temp;
        SM_ASSERT(target != NULL);
        len = path_to_top(me, target, path);
        idx = path_find(path, len, me->state);
        /* Transicao inicial so pode ir para um descendente do composto. */
        SM_ASSERT(idx != PATH_NOT_FOUND);
        enter_down(me, path, idx);
        me->state = target;
    }
}

/*
 * Transicao de `source` (o estado que tratou o evento: a folha ou um
 * ancestral dela) para `target`. O ancestral comum (LCA) e o estado mais
 * profundo que e ancestral-ou-o-proprio `source` E ancestral proprio de
 * `target`. Com isso:
 *   - irmaos/primos: LCA e o pai comum;
 *   - auto-transicao (source == target): LCA e o pai -- sai e entra de novo;
 *   - alvo dentro de source: LCA e o proprio source (transicao local);
 *   - alvo e ancestral de source: LCA e o pai do alvo -- sai e entra no alvo.
 * Sai da folha ate o LCA (exclusive), entra do LCA ate o alvo, e segue as
 * transicoes iniciais do alvo.
 */
static void transition(hsm_t *const me, hsm_state_fn source,
                       hsm_state_fn target)
{
    hsm_state_fn path[HSM_MAX_DEPTH];
    hsm_state_fn s;
    size_t len;
    size_t idx;
#if (SM_TRACE == 1)
    const hsm_state_fn previous = me->state;
#endif

    SM_ASSERT(target != NULL);
    SM_ASSERT(target != hsm_top);

    len = path_to_top(me, target, path);

    /* 1) estados abaixo de source: sempre saem. */
    for (s = me->state; s != source; s = parent_of(me, s)) {
        leave(me, s);
    }

    /* 2) de source ate o LCA (exclusive). hsm_top esta em path, entao o
     *    laco termina no maximo nele -- e hsm_top nunca e deixado. */
    idx = path_find(path, len, s);
    while (idx == PATH_NOT_FOUND) {
        leave(me, s);
        s   = parent_of(me, s);
        idx = path_find(path, len, s);
    }

    /* 3) do LCA ate o alvo, e as transicoes iniciais dele. */
    enter_down(me, path, idx);
    me->state = target;
    follow_initial(me);

#if (SM_TRACE == 1)
    hsm_trace(previous, me->state);
#endif
}
