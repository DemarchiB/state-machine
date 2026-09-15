# ADR-0001: Fila de eventos fica fora da FSM/HSM

- **Status:** aceito
- **Data:** 2026-09-14
- **Substitui:** —

## Contexto
A biblioteca `fsm` precisa ser testável no host, chamando `fsm_dispatch` diretamente e conferindo o estado resultante, sem RTOS nem mock de fila. Se o mecanismo de enfileiramento (FreeRTOS, POSIX, outro) entrasse dentro do dispatch da FSM, toda máquina — mesmo uma que nunca vai rodar como Active Object — ficaria acoplada a esse mecanismo desde a primeira linha, e testar a lógica de transição exigiria subir uma fila de verdade só para confirmar `δ(estado, evento)`.

## Decisão
A fila de eventos não faz parte de `fsm.h`/`fsm.c` (nem do futuro `hsm.h`/`hsm.c`). `fsm_dispatch` é sempre síncrono: quem chama decide de onde o evento veio — outro trecho de código, um teste, ou (mais tarde) uma task consumindo uma fila por fora. A fila, o backend plugável (`post`/`receive`) e a task entram só na camada de Active Object, num repositório futuro, associando FSM/HSM + fila + task.

## Alternativas consideradas
- Fila embutida em `fsm_dispatch`, com um backend plugável desde já: descartada porque exige mock/stub de fila até para o teste mais simples no host, e acopla a API pública a um conceito que nem toda máquina usa.
- Fila global única, compartilhada por todas as máquinas do processo: descartada por esconder a relação entre cada máquina e sua fonte de eventos, e por dificultar testar uma máquina isolada das outras.

## Consequências
`fsm_dispatch` nunca bloqueia nem depende de RTOS — testável em CTest puro, sem hardware, sem mock. Em troca, quem quiser rodar a máquina de forma orientada a evento (por exemplo, reagindo a uma ISR) precisa da camada de Active Object por cima; não existe atalho dentro da FSM para isso.
