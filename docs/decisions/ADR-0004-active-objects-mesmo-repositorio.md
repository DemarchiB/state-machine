# ADR-0004: Active Objects no mesmo repositório, com porta escolhida no link

- **Status:** aceito
- **Data:** 2026-09-19
- **Substitui:** — (revê a localização de Active Objects prevista em ADR-0001 e ADR-0003)

## Contexto
ADR-0001 e ADR-0003 previam Active Objects num repositório separado, por depender de fila, task e RTOS. Na prática, o Active Object é pequeno, depende diretamente de `sm_event_t` e de `fsm`/`hsm`, e quem usa uma máquina em firmware com RTOS quase sempre quer os três juntos. O que ADR-0001 protege — `fsm`/`hsm` sem nenhuma dependência de fila — se preserva com um alvo separado, sem precisar de outro repositório.

Também era preciso decidir como trocar o backend de fila/task (FreeRTOS, POSIX, Windows): por tabela de ponteiros de função em tempo de execução ou por escolha no build.

## Decisão
- Active Objects é o componente `sm_ao` deste repositório (`include/sm/ao.h`, `src/ao.c`). `sm_fsm` e `sm_hsm` não dependem dele nem incluem nada dele.
- O backend é uma **porta escolhida no link**: `ports/<porta>/ao_port.h` + `ao_port.c`, selecionada pela variável de CMake `SM_AO_PORT` (`posix`, `win32`, `freertos`, `none`). O contrato interno entre `ao.c` e as portas está em `src/ao_port_api.h`.
- O evento é **copiado** para a fila, em posições de tamanho fixo (`AO_EVENT_SIZE_MAX`), sem pool nem contagem de referência. Postar nunca bloqueia; fila cheia volta como `AO_ERR_QUEUE_FULL`.
- `ao_start`, `ao_post`, `ao_post_from_isr` e `ao_stop` retornam `ao_err_t`. Não contraria ADR-0002: fila cheia e falha ao criar task são condições reais de execução, não violação de contrato. Violação de contrato (ponteiro nulo, sinal reservado, evento maior que o máximo) continua em `SM_ASSERT`.

## Alternativas consideradas
- Repositório separado para Active Objects: descartada — mesma duplicação de infraestrutura que ADR-0003 evitou para a HSM, sem ganho de isolamento que um alvo separado já não dê.
- Porta por tabela de ponteiros de função em tempo de execução: descartada — nunca há duas portas no mesmo binário, e ponteiro de função quebra o grafo de chamadas estático usado em análise de pilha e rastreabilidade.
- Fila de ponteiros para eventos em pool, com contagem de referência (modelo do QP): descartada por ora — mais memória de controle e regras de posse para a aplicação seguir; a cópia por valor é suficiente para eventos pequenos.

## Consequências
Um build tem uma porta só; testar duas portas exige dois diretórios de build. Todo evento ocupa `AO_EVENT_SIZE_MAX` bytes na fila, então evento grande encarece todas as posições — payload grande deve ir por referência a um buffer da aplicação. A porta `freertos` exige `configSUPPORT_STATIC_ALLOCATION = 1` e `INCLUDE_vTaskSuspend = 1`.
