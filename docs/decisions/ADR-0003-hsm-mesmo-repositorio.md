# ADR-0003: HSM no mesmo repositório que a FSM

- **Status:** aceito
- **Data:** 2026-09-15
- **Substitui:** —

> Nota (2026-09-19): a localização de Active Objects, citada abaixo como repositório separado, foi revista em [ADR-0004](ADR-0004-active-objects-mesmo-repositorio.md). A decisão sobre a HSM continua valendo.

## Contexto
A implementação de HSM precisava de um lugar: este repositório, junto da FSM plana, ou um repositório novo e separado.

FSM e HSM compartilham infraestrutura real: o tipo de evento, a convenção de sinais reservados, o contrato por assert (ADR-0002), a decisão de manter a fila fora da máquina (ADR-0001) e o teste em host via CTest. Há precedente direto: o QP/C tem as duas variantes no mesmo módulo (QEP), sobre o mesmo tipo de evento. Em C, um componente a mais no repositório não pesa para quem só usa a parte plana — só entra no build quem referenciar o alvo.

## Decisão
HSM é um componente irmão da FSM neste repositório (`include/sm/hsm.h`/`src/hsm.c`, alvo `sm_hsm`), não um repositório próprio. O repositório se chama `state-machine`, para não sugerir que cobre só a variante plana.

## Alternativas consideradas
- Repositório novo para HSM: descartada — duplicaria a infraestrutura de projeto (guia, `AGENTS.md`, `ARCHITECTURE.md`, workflow) ou faria um repositório depender do outro por submódulo/`FetchContent`, sem benefício; e obrigaria quem usa as duas variantes a gerenciar duas dependências.

## Consequências
O nome do alvo diz o que ele é (`sm_fsm`, `sm_hsm`), não o nome do repositório. Se a HSM crescer a ponto de a mistura atrapalhar de fato — API muito maior, ciclo de release diferente —, separar depois é reversível.
