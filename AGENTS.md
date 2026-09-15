# AGENTS.md

Biblioteca em C de máquinas de estado finitas (FSM), base para HSM e Active Objects futuros. Uma contribuição está pronta quando os testes em host passam e nenhum aviso novo aparece no build. Prioridade: API pequena e simples de adotar via submódulo em outros projetos — evitar peso documental que não ajuda quem só vai consumir a lib.

## Convenções
Segue `docs/guide/` (submódulo). No início de toda tarefa, leia
`docs/guide/PROJECT_GUIDE.md` — ele diz o que mais ler.

- Domínios aplicáveis: engenharia, ia, specs, c-embarcado, c-build-e-analise, firmware.
- Idioma: documentação em pt-BR. Identificadores da biblioteca (`fsm_*`, `event_t`, `fsm_t`) em inglês/genérico, por serem termos de framework; identificadores de aplicação (exemplos, testes, máquinas concretas) em pt-BR. Comentários em pt-BR. Commits em pt-BR.
- Convenção de caixa: `snake_case` para funções e variáveis; tipos com sufixo `_t`. Nenhum `PascalCase` neste repositório (desvio deliberado do exemplo ilustrativo de `docs/guide/templates/modulo-c.md`, que é só um exemplo de convenção — o guia deixa a escolha de caixa a critério do projeto).

## Comandos
| Ação | Comando | Diretório |
| --- | --- | --- |
| Configurar | `cmake -S . -B build` | raiz |
| Build | `cmake --build build` | raiz |
| Testes | `ctest --test-dir build` | raiz |
| Lint / análise | `<a definir: nenhuma ferramenta adotada ainda>` | — |
| Links da documentação | `python docs/guide/tools/verificar.py` | raiz |

Nenhum destes comandos foi executado até agora neste repositório — `<a verificar>` até a primeira execução confirmar que funcionam (não há `src/`/`tests/` com conteúdo ainda).

## Onde fica o quê
- `docs/decisions/` — ADR só quando a decisão for difícil de reverter e afetar mais de um ponto da lib (`docs/guide/templates/adr.md`, critério de quando registrar). A primeira é `ADR-0001-fila-fora-da-fsm.md`. Spec formal (`docs/specs/`) não é usada por padrão nesta lib — API pequena o suficiente pra não precisar, por ora.
- `include/fsm.h` — interface pública da biblioteca (`fsm_t`, `event_t`, `fsm_dispatch`); ver ADR-0001 pro que ela deliberadamente não tem.
- `src/`, `tests/`, `examples/` — vazios até a primeira implementação real existir.

## Restrições críticas
- `fsm_dispatch` não aloca memória dinamicamente e não bloqueia.
- A fila de eventos não entra neste repositório — ver ADR-0001. Se algum código aqui precisar de fila, é sinal de que ele pertence à camada de Active Object, não a este repositório.
- Agente trabalha em branch própria (`docs/workflow.md`) e não executa merge, push para `main` nem reescrita de histórico.

## Ao terminar
1. Rodar os comandos de build e testes que se aplicam à mudança.
2. Atualizar a documentação afetada, uma vez, com o comportamento já verificado.
3. Entregar o resumo da mudança no formato de
   `docs/guide/practices/engenharia.md`, Seção *Processo de uma mudança*.
