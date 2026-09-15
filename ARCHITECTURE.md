# Arquitetura

## Visão geral
Biblioteca em C (C99) de máquinas de estado. Hoje contém a FSM plana (`include/fsm.h`/`src/fsm.c`), pronta e testada; a HSM entra como um segundo componente no mesmo repositório, não em repositório próprio — decisão e motivo em [ADR-0003](docs/decisions/ADR-0003-hsm-mesmo-repositorio.md). Active Objects continua em repositório futuro e separado, por depender de fila/task/RTOS (fora do escopo síncrono desta lib — ver ADR-0001). Roda em host (testes, CTest) e em firmware embarcado, sem depender de RTOS nem de alocação dinâmica.

## Dependências e interfaces externas
| Dependência | Uso | Limite |
| --- | --- | --- |
| CMake | Build e testes em host | Só a raiz do repositório; nenhum arquivo de biblioteca inclui CMake |
| CTest | Testes em host, sem hardware | `tests/`; a biblioteca em si não depende de framework de teste nenhum |

Nenhuma dependência de RTOS, driver ou hardware específico nesta fase — é um ponto deliberado (ver *Restrições a preservar*), não uma omissão.

## Restrições a preservar
- **A fila de eventos não entra nesta biblioteca** — `fsm_dispatch` é sempre síncrono, chamado diretamente por quem tem o evento. Fila, task e backend de RTOS/POSIX ficam na camada de Active Object, num repositório futuro. Decisão registrada em [ADR-0001](docs/decisions/ADR-0001-fila-fora-da-fsm.md).
- **Sem alocação dinâmica.** Toda instância de `fsm_t` (e, quando existir, `hsm_t`) é alocada por quem a usa (estática ou em pilha); a biblioteca nunca chama `malloc`.
- **API pública em C99 puro**, sem extensão de compilador específico, para não amarrar a um toolchain só.
- **Contrato validado por assert, não por retorno de erro.** `fsm_init`/`fsm_dispatch` são `void`; ponteiro nulo é bug de quem chama, verificado por `FSM_ASSERT`. Decisão registrada em [ADR-0002](docs/decisions/ADR-0002-fsm-dispatch-void-assert.md).
- **FSM e HSM compartilham `event_t` e a convenção de sinais reservados** (`SIG_ENTRY`/`SIG_EXIT`/`SIG_USER`) — é justamente essa infraestrutura comum que justifica o mesmo repositório em vez de dois; ver ADR-0003.

## Pontos não determinados
- Se `event_t` vai ganhar um campo de payload genérico já nesta fase, ou só quando a primeira máquina de aplicação precisar de um evento com dado associado (composição: struct derivada com `event_t` como primeiro membro, mesmo idioma de `docs/guide/templates/modulo-c.md`, Seção *Polimorfismo*).
- Se `FSM_ASSERT` ganha um handler customizado que força estado seguro em vez de `abort()` padrão em build de release: fica para quando o projeto entrar em contexto de safety, não decidido agora.
- Se o subconjunto MISRA definido no guia (`practices/c-embarcado.md`) é adotado já nesta biblioteca ou só quando o projeto entrar em contexto de safety: `<a definir>`.
- Versão mínima de CMake exigida: `<a verificar: nenhum build foi rodado ainda para confirmar>`.
- Nome e assinatura exatos do tipo/funções de HSM (`hsm_t` próprio? reaproveita `fsm_t` com campo de hierarquia?) — decisão da Fase 4, ainda não tomada.
