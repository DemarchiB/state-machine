# state-machine

Biblioteca em C99 de máquinas de estado para firmware embarcado e host: máquina plana (`fsm`), hierárquica (`hsm`) e Active Object (`ao`), que roda uma dessas máquinas numa task própria com fila de eventos. Sem alocação dinâmica, sem recursão; `fsm` e `hsm` não dependem de RTOS nem de sistema operacional.

## Pré-requisitos
- Compilador C99 (testado com GCC 13 no Linux e MinGW-w64 13 para Windows)
- CMake 3.16 ou mais novo
- Para `ao`: FreeRTOS (com alocação estática), POSIX threads ou Windows Vista+

## Como começar
```
git clone --recurse-submodules https://github.com/DemarchiB/state-machine.git
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Com gerador multi-config (Visual Studio), informe a configuração: `cmake --build build --config Debug` e `ctest --test-dir build -C Debug`.

## Usar em outro projeto
Como submódulo, com `add_subdirectory` (testes e exemplos ficam desligados automaticamente):

```cmake
set(SM_AO_PORT freertos)                 # ou posix, win32, none
add_subdirectory(libs/state-machine)
target_link_libraries(firmware PRIVATE sm::fsm sm::hsm sm::ao)
```

| Variável de CMake | Padrão | Efeito |
| --- | --- | --- |
| `SM_AO_PORT` | automático: `win32` no Windows, `posix` em Unix, `none` em alvo sem SO | Porta do Active Object; `none` não cria `sm::ao` |
| `SM_AO_EVENT_SIZE_MAX` | `16` | Maior evento postável (bytes); tamanho de cada posição de fila |
| `SM_HSM_MAX_DEPTH` | `8` | Profundidade máxima da hierarquia, contando `hsm_top` |
| `SM_BUILD_TESTS`, `SM_BUILD_EXAMPLES` | `ON` só como projeto principal | Testes e exemplos |
| `SM_WARNINGS_AS_ERRORS` | `ON` só como projeto principal | `-Werror`/`/WX` nos alvos desta biblioteca |
| `SM_FREERTOS_SIMULATOR` | `OFF` | Com `SM_AO_PORT=freertos`, baixa o FreeRTOS-Kernel e testa a porta no simulador POSIX (Linux) |

Definições de compilação opcionais, iguais no projeto inteiro: `SM_ASSERT(cond)` (padrão `assert`) e `SM_TRACE=1` (liga os hooks `fsm_trace`/`hsm_trace`, que a aplicação implementa).

A porta `freertos` usa o alvo `freertos_kernel` do CMake do FreeRTOS-Kernel e exige no `FreeRTOSConfig.h`: `configSUPPORT_STATIC_ALLOCATION 1` e `INCLUDE_vTaskSuspend 1`.

## Em uma página

**FSM** — cada estado é uma função; a ação de entrada mora em `SM_SIG_ENTRY`:

```c
#include "sm/fsm.h"

enum { EV_MOEDA = SM_SIG_USER, EV_EMPURRAR };

static void st_liberada(fsm_t *const me, const sm_event_t *const e);

static void st_travada(fsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY: fecha_trava();             break;
    case EV_MOEDA:     fsm_tran(me, st_liberada); break;
    default:                                      break; /* ignorado */
    }
}

fsm_t catraca;
fsm_init(&catraca, st_travada);           /* dispara a entrada do estado inicial */
fsm_dispatch(&catraca, &ev);              /* síncrono, até o fim */
```

**HSM** — o `default` devolve o pai; o que o filho não trata sobe:

```c
#include "sm/hsm.h"

static hsm_status_t st_falha_segura(hsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY: fecha_trava();                     return HSM_HANDLED;
    case EV_RESET:     return hsm_tran(me, st_operacional);
    default:           return hsm_super(me, hsm_top);     /* estado de 1º nível */
    }
}

static hsm_status_t st_falha_sensor(hsm_t *const me, const sm_event_t *const e)
{
    switch (e->sig) {
    case SM_SIG_ENTRY: registra_falha();                  return HSM_HANDLED;
    default:           return hsm_super(me, st_falha_segura);
    }
}
```

Estado composto trata `SM_SIG_INIT` com `hsm_tran` para um filho (transição inicial). Transição sai dos estados até o ancestral comum e entra até o alvo, na ordem de construtores/destrutores aninhados.

**Active Object** — a mesma máquina, numa task, recebendo eventos de qualquer contexto:

```c
#include "sm/ao.h"

typedef struct { ao_t super; fsm_t sm; } catraca_ao_t;   /* ao_t primeiro */

static void setup(ao_t *const me)   { fsm_init(&((catraca_ao_t *)me)->sm, st_travada); }
static void handler(ao_t *const me, const sm_event_t *const e)
                                    { fsm_dispatch(&((catraca_ao_t *)me)->sm, e); }

static catraca_ao_t catraca;
static ao_msg_t     fila[8];
static StackType_t  pilha[256];
static const ao_port_cfg_t cfg = { fila, 8, pilha, 256, 2, "catraca" };  /* porta freertos */

ao_init(&catraca.super, setup, handler);
ao_start(&catraca.super, &cfg);
ao_post_from_isr(&catraca.super, &ev, sizeof ev);   /* copia o evento; nunca bloqueia */
```

Evento com dado é uma struct com `sm_event_t` como primeiro membro; `ao_post` copia `sizeof` dela, até `SM_AO_EVENT_SIZE_MAX`.

Exemplos completos e executáveis em `examples/` (`fsm_catraca`, `hsm_catraca`, `ao_catraca`); os testes em `tests/` mostram o comportamento esperado em cada caso.

## Documentação
- Arquitetura: [ARCHITECTURE.md](ARCHITECTURE.md)
- Instruções para agentes de IA: [AGENTS.md](AGENTS.md)
- Convenções de projeto: [docs/guide/PROJECT_GUIDE.md](docs/guide/PROJECT_GUIDE.md)
- Workflow de revisão: [docs/workflow.md](docs/workflow.md)
- Decisões: [docs/decisions/](docs/decisions/)

## Licença
`<a definir>`
