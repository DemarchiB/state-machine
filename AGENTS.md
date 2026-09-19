# AGENTS.md

Biblioteca em C99 de máquinas de estado: `fsm` (plana), `hsm` (hierárquica) e `ao` (Active Object, com portas FreeRTOS/POSIX/Win32). Uma contribuição está pronta quando build e testes em host passam sem aviso novo — os alvos deste projeto compilam com aviso como erro — e o comportamento novo tem teste.

## Convenções
Segue `docs/guide/` (submódulo). No início de toda tarefa, leia
`docs/guide/PROJECT_GUIDE.md` — ele diz o que mais ler.

- Domínios aplicáveis: engenharia, ia, specs, c-embarcado, c-build-e-analise, firmware.
- Desvios do guia: construtores e despacho de `fsm`/`hsm` retornam `void` e validam por `SM_ASSERT`, não por código de erro — ADR-0002. Caixa `snake_case` para funções e variáveis, tipos com sufixo `_t` (não o PascalCase ilustrativo de `templates/modulo-c.md`). Máquinas dos exemplos são documentadas por tabela/árvore no próprio `.c`, sem design-doc: são exemplo de uso, não comportamento de produto.
- Idioma: documentação e comentários em pt-BR; identificadores da biblioteca em inglês (`fsm_dispatch`, `sm_event_t`); identificadores de exemplos e testes em pt-BR; commits em pt-BR.
- Todo símbolo público tem prefixo `sm_`/`SM_`, `fsm_`, `hsm_` ou `ao_`; headers ficam em `include/sm/` e são incluídos como `"sm/<nome>.h"` — ADR-0005.
- Os testes usam `CHECK` de `tests/check.h`, nunca `assert` (que some em Release).

## Comandos
| Ação | Comando | Diretório |
| --- | --- | --- |
| Configurar | `cmake -S . -B build` | raiz |
| Build | `cmake --build build` | raiz |
| Testes | `ctest --test-dir build --output-on-failure` | raiz |
| Testes da porta freertos (Linux) | `cmake -S . -B build-freertos -DSM_AO_PORT=freertos -DSM_FREERTOS_SIMULATOR=ON && cmake --build build-freertos && ctest --test-dir build-freertos` | raiz |
| Lint / análise | `<a definir: nenhuma ferramenta adotada>` | — |
| Links da documentação | `python docs/guide/tools/verificar.py` | raiz |

Com gerador multi-config (Visual Studio): `--config Debug` no build e `-C Debug` no ctest. Build com MSVC: `<a verificar: não executado desde a entrada de hsm/ao>`.

## Onde fica o quê
- `src/ao_port_api.h` — contrato interno entre `ao.c` e as portas; porta nova implementa só ele, em `ports/<porta>/`.
- `tests/freertos/` — `FreeRTOSConfig.h` mínimo e teste da porta `freertos` no simulador; só é configurado com `SM_FREERTOS_SIMULATOR=ON`.
- `docs/decisions/` — ADRs 0001 a 0005; ler antes de mexer em limite entre componentes, retorno de erro, porta ou nomes públicos.

## Restrições críticas
- `src/fsm.c` e `src/hsm.c` não incluem nada de fila, thread ou RTOS — ADR-0001.
- Sem `malloc`, sem recursão, sem VLA em nenhum componente.
- Função pública nova só retorna erro para condição real de execução (fila cheia, porta); violação de contrato é `SM_ASSERT` — ADR-0002, ADR-0004.
- Agente trabalha em branch própria (`docs/workflow.md`) e não executa merge, push para `main` nem reescrita de histórico.

## Ao terminar
1. Rodar build e testes; se tocou em `ao.c` ou `ports/freertos/`, também os testes da porta freertos.
2. Atualizar a documentação afetada, uma vez, com o comportamento já verificado.
3. Entregar o resumo da mudança no formato de
   `docs/guide/practices/engenharia.md`, Seção *Processo de uma mudança*.
