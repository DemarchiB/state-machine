# Arquitetura

## Visão geral
Biblioteca em C99 de máquinas de estado para firmware embarcado e host: máquina plana (`fsm`), máquina hierárquica (`hsm`) e Active Object (`ao`), que executa uma dessas máquinas numa task própria, alimentada por uma fila de eventos. `fsm` e `hsm` são síncronas e não dependem de RTOS, sistema operacional nem alocação dinâmica; só `ao` depende de uma porta (FreeRTOS, POSIX ou Win32), escolhida no build.

## Domínios e componentes
| Componente | Responsabilidade | Caminho |
| --- | --- | --- |
| Comum (`sm_common`) | Tipo de evento, sinais reservados, assert de contrato | `include/sm/sm_event.h`, `include/sm/sm_assert.h` |
| FSM (`sm_fsm`) | Máquina plana: estado como função, entrada/saída, transição | `include/sm/fsm.h`, `src/fsm.c` |
| HSM (`sm_hsm`) | Máquina hierárquica: evento sobe ao pai, transição pelo ancestral comum, transição inicial | `include/sm/hsm.h`, `src/hsm.c` |
| Active Object (`sm_ao`) | Cópia de eventos para a fila e laço de despacho, independentes de porta | `include/sm/ao.h`, `src/ao.c` |
| Portas do Active Object | Fila e task sobre FreeRTOS, POSIX ou Win32 | `ports/<porta>/ao_port.h`, `ports/<porta>/ao_port.c` |

Dependência em um sentido só: `sm_ao` → `sm_common`; `sm_fsm` e `sm_hsm` → `sm_common`. `sm_ao` não conhece `fsm` nem `hsm`: a aplicação liga a máquina ao objeto por duas funções (`ao_setup_fn`, `ao_handler_fn`).

## Fluxos principais
Despacho síncrono: quem tem o evento chama `fsm_dispatch`/`hsm_dispatch`, que executa até o fim (run-to-completion) e retorna. Na `hsm`, o evento começa na folha ativa e sobe pelos pais até um estado tratar; uma transição sai dos estados até o ancestral comum com a origem, entra até o alvo e segue as transições iniciais.

Active Object: ISR ou outra task → `ao_post`/`ao_post_from_isr` (cópia do evento, sem bloquear) → fila da porta → task do objeto → `ao_handler_fn` → `fsm_dispatch`/`hsm_dispatch`. Uma única task consome a fila, e é isso que serializa os eventos: não há mutex em volta da máquina.

## Dependências e interfaces externas
| Dependência | Uso | Limite |
| --- | --- | --- |
| CMake ≥ 3.16 | Build, testes e seleção de porta | Nenhum arquivo `.c`/`.h` depende do CMake; as opções chegam como definições de compilação |
| FreeRTOS-Kernel | Porta `freertos` | Só `ports/freertos/` inclui headers do FreeRTOS; o alvo `freertos_kernel` é fornecido por quem consome |
| pthreads / API Win32 | Portas `posix` e `win32` (host) | Só `ports/posix/` e `ports/win32/` |
| FreeRTOS-Kernel V11.1.0 (simulador POSIX) | Teste da porta `freertos` em host | Só `tests/freertos/`, baixado por `FetchContent` com commit fixado, e só com `SM_FREERTOS_SIMULATOR=ON` |

## Restrições a preservar
- **`fsm` e `hsm` não conhecem fila, task nem RTOS** — nem um `#include`. ([ADR-0001](docs/decisions/ADR-0001-fila-fora-da-fsm.md))
- **Sem alocação dinâmica e sem recursão** em nenhum componente. Fila, pilha e objetos são memória da aplicação; a profundidade da `hsm` é limitada por `HSM_MAX_DEPTH`.
- **Contrato por assert, erro de execução por retorno.** Ponteiro nulo, sinal reservado e evento grande demais são violação de contrato (`SM_ASSERT`); fila cheia e falha da porta voltam como `ao_err_t`. ([ADR-0002](docs/decisions/ADR-0002-fsm-dispatch-void-assert.md), [ADR-0004](docs/decisions/ADR-0004-active-objects-mesmo-repositorio.md))
- **Porta escolhida no link, uma por build**, nunca por ponteiro de função em tempo de execução. ([ADR-0004](docs/decisions/ADR-0004-active-objects-mesmo-repositorio.md))
- **Todo símbolo público tem prefixo** (`sm_`, `fsm_`, `hsm_`, `ao_`) e os headers ficam em `include/sm/`. ([ADR-0005](docs/decisions/ADR-0005-prefixos-e-headers.md))
- **API pública em C99**, sem extensão de compilador; headers incluíveis por C++.
- **`AO_EVENT_SIZE_MAX`, `HSM_MAX_DEPTH` e `SM_TRACE` são iguais em todas as unidades de tradução** que os usam — definidos no build, nunca num arquivo isolado.

## Pontos não determinados
- Handler de `SM_ASSERT` que leva o produto a estado seguro em release, em vez de sumir com `NDEBUG`: `<a definir>` — depende do uso em contexto de safety.
- Adoção do subconjunto MISRA do guia nesta biblioteca: `<a definir>`.
- Build e testes com MSVC (Visual Studio): `<a verificar: a porta win32 foi compilada com MinGW e executada no Wine; MSVC não foi executado>`.
