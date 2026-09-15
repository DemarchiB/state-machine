# ADR-0003 — HSM no mesmo repositório que a FSM

## Status
Aceita

## Contexto
A Fase 4 do estudo (HSM) precisava decidir onde a implementação de HSM ia morar: neste repositório, junto da FSM plana já implementada, ou num repositório novo e separado (era a suposição inicial, registrada informalmente nas notas da Fase 4).

FSM e HSM não são duas bibliotecas incidentalmente parecidas — compartilham infraestrutura real: `event_t`, a convenção de sinais reservados (`SIG_ENTRY`/`SIG_EXIT`/`SIG_USER`), o contrato por `FSM_ASSERT` (ADR-0002), a decisão de manter a fila fora da lib (ADR-0001), e a mesma filosofia de teste em host via CTest. Existe precedente direto de projetar assim: o QP/C (referência arquitetural já usada no ADR-0002) tem os dois no mesmo módulo (QEP) — `QMsm` (plana) e `QHsm` (hierárquica), ambos sobre o mesmo `QEvt`.

Em C, uma biblioteca extra no mesmo repositório não é peso para quem só usa a parte plana: só entra no build quem referenciar `hsm.c`/`hsm.h` no próprio `CMakeLists.txt` (diferente de um pacote de linguagem com gerenciador de dependências, onde a dependência inteira entra junto).

## Decisão
HSM é implementada neste repositório, como um componente irmão da FSM (`include/hsm.h`/`src/hsm.c`, quando tiver conteúdo real — não antes), não em repositório próprio. Consequência direta: o repositório deixa de se chamar `fsm` e passa a se chamar `state-machine`, para não sugerir que só cobre a variante plana.

Active Objects **não** entra nesta decisão — continua em repositório futuro e separado, porque depende de fila/task/backend de RTOS, o que é justamente o que ADR-0001 mantém fora desta biblioteca.

## Alternativas descartadas
- **Repositório novo para HSM** (suposição inicial): descartada. Duplicaria toda a infraestrutura de projeto (guia como submódulo, `AGENTS.md`, `ARCHITECTURE.md`, `docs/workflow.md`, CI) só para reusar um `event_t` e uma convenção de sinais já prontos aqui — ou faria o repositório de HSM depender deste via submódulo/`FetchContent`, uma indireção sem benefício claro. Também forçaria quem precisa das duas variantes no mesmo projeto (comum — algumas máquinas ficam planas, só a complexa vira hierárquica) a gerenciar duas dependências em vez de uma.

## Consequências
- Repositório renomeado de `fsm` para `state-machine` no GitHub; `project()` no `CMakeLists.txt` raiz segue o mesmo nome. O target da biblioteca plana continua se chamando `fsm` (é o que ele é); o de HSM, quando existir, deve se chamar `hsm` pelo mesmo motivo — não `state-machine`, que é o nome do repositório/projeto, não de um target específico.
- O submódulo em `Estudo/` mantém o path local `fsm` (não foi renomeado para `state-machine`) — evita a cirurgia de `git submodule deinit`/re-add, que arrisca perder trabalho não commitado sem necessidade real (só o nome do repositório remoto muda, via `git submodule set-url`).
- Se, na prática, a HSM crescer a ponto de a mistura virar confusão real (não hipotética) — API muito maior, ciclo de release diferente, contexto de safety divergente — separar depois é reversível: este ADR registra a decisão tomada agora, não uma garantia permanente.
