# Arquitetura

## Visão geral
Biblioteca em C (C99) de máquinas de estado finitas (FSM) — base para as bibliotecas de HSM e Active Objects que vêm depois, em repositórios próprios. Roda em host (testes, CTest) e em firmware embarcado, sem depender de RTOS nem de alocação dinâmica. Projeto de um componente só: esta biblioteca não tem sub-módulos internos além de `fsm.h`/`fsm.c`.

## Dependências e interfaces externas
| Dependência | Uso | Limite |
| --- | --- | --- |
| CMake | Build e testes em host | Só a raiz do repositório; nenhum arquivo de biblioteca inclui CMake |
| CTest | Testes em host, sem hardware | `tests/`; a biblioteca em si não depende de framework de teste nenhum |

Nenhuma dependência de RTOS, driver ou hardware específico nesta fase — é um ponto deliberado (ver *Restrições a preservar*), não uma omissão.

## Restrições a preservar
- **A fila de eventos não entra nesta biblioteca** — `fsm_dispatch` é sempre síncrono, chamado diretamente por quem tem o evento. Fila, task e backend de RTOS/POSIX ficam na camada de Active Object, num repositório futuro. Decisão registrada em [ADR-0001](docs/decisions/ADR-0001-fila-fora-da-fsm.md).
- **Sem alocação dinâmica.** Toda instância de `fsm_t` é alocada por quem a usa (estática ou em pilha); a biblioteca nunca chama `malloc`.
- **API pública em C99 puro**, sem extensão de compilador específico, para não amarrar a um toolchain só.

## Pontos não determinados
- Se `event_t` vai ganhar um campo de payload genérico já nesta fase, ou só quando a primeira máquina de aplicação precisar de um evento com dado associado (composição: struct derivada com `event_t` como primeiro membro, mesmo idioma de `docs/guide/templates/modulo-c.md`, Seção *Polimorfismo*).
- Convenção de códigos de erro além de `FSM_OK`/`FSM_ERR_PARAM`: `<a definir>` — ainda não apareceu um caso de erro que precise de um código próprio.
- Se o subconjunto MISRA definido no guia (`practices/c-embarcado.md`) é adotado já nesta biblioteca ou só quando o projeto entrar em contexto de safety: `<a definir>`.
- Versão mínima de CMake exigida: `<a verificar: nenhum build foi rodado ainda para confirmar>`.
