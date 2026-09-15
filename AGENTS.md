# AGENTS.md

Biblioteca em C de máquinas de estado: FSM plana (pronta, `include/fsm.h`/`src/fsm.c`) e HSM (em andamento), no mesmo repositório — ver [ADR-0003](docs/decisions/ADR-0003-hsm-mesmo-repositorio.md). Active Objects continua em repositório futuro e separado. Uma contribuição está pronta quando os testes em host passam e nenhum aviso novo aparece no build. Prioridade: API pequena e simples de adotar via submódulo em outros projetos — evitar peso documental que não ajuda quem só vai consumir a lib.

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

Configurar/Build/Testes confirmados funcionando (Bruno, gerador Visual Studio — multi-config precisa de `--config Debug`/`-C Debug` nos dois últimos, ver README). Lint segue `<a definir>`; link-checker segue `<a verificar>` até a primeira execução real.

## Onde fica o quê
- `docs/decisions/` — ADR só quando a decisão for difícil de reverter e afetar mais de um ponto da lib (`docs/guide/templates/adr.md`, critério de quando registrar). `ADR-0001-fila-fora-da-fsm.md` (fila fora da lib), `ADR-0002-fsm-dispatch-void-assert.md` (contrato por assert, não por retorno de erro), `ADR-0003-hsm-mesmo-repositorio.md` (HSM como componente irmão, não repositório novo). Spec formal (`docs/specs/`) não é usada por padrão nesta lib — API pequena o suficiente pra não precisar, por ora.
- `include/fsm.h` / `src/fsm.c` — FSM plana: `fsm_t`, `event_t`, `fsm_init`, `fsm_dispatch`, `fsm_tran` (este último `static inline` no header); ver ADR-0001 pro que ela deliberadamente não tem, ADR-0002 pro porquê de ser `void`.
- `include/hsm.h` / `src/hsm.c` — ainda não existem; nascem com a Fase 4, com o primeiro conteúdo real (mesma regra de sempre: nada de esqueleto vazio antes da hora).
- `examples/catraca/` — exemplo de uso da FSM (`catraca.c`/`catraca.h`/`main.c`); `tests/test_catraca.c` — teste em host real (CTest), cobre a sequência completa de transições, um evento ignorado de propósito, e trace (`FSM_TRACE`) ligado só neste alvo.

## Restrições críticas
- `fsm_dispatch` não aloca memória dinamicamente e não bloqueia.
- A fila de eventos não entra neste repositório — ver ADR-0001. Se algum código aqui precisar de fila, é sinal de que ele pertence à camada de Active Object, não a este repositório.
- `fsm_init`/`fsm_dispatch` são `void`; nenhuma função pública nova entra retornando código de erro sem repetir a discussão de ADR-0002 primeiro.
- Agente trabalha em branch própria (`docs/workflow.md`) e não executa merge, push para `main` nem reescrita de histórico.

## Ao terminar
1. Rodar os comandos de build e testes que se aplicam à mudança.
2. Atualizar a documentação afetada, uma vez, com o comportamento já verificado.
3. Entregar o resumo da mudança no formato de
   `docs/guide/practices/engenharia.md`, Seção *Processo de uma mudança*.
