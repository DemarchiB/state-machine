# ADR-0005: Prefixo em todo símbolo público e headers em `include/sm/`

- **Status:** aceito
- **Data:** 2026-09-19
- **Substitui:** —

## Contexto
A primeira versão expunha `event_t`, `SIG_ENTRY`/`SIG_EXIT`/`SIG_USER` e `FSM_ASSERT`, e headers com nomes genéricos na raiz de `include/`. `event_t` e `SIG_*` são nomes comuns o bastante para colidir com código de RTOS, HAL ou da própria aplicação de quem adota a biblioteca, e o guia de convenções exige prefixo de módulo em todo símbolo público. Com `hsm` e `ao` entrando, o evento e o assert deixaram de ser da FSM e passaram a ser compartilhados.

## Decisão
- Símbolos compartilhados com prefixo `sm_`/`SM_`: `sm_event_t`, `SM_SIG_EMPTY`, `SM_SIG_ENTRY`, `SM_SIG_EXIT`, `SM_SIG_INIT`, `SM_SIG_USER`, `SM_ASSERT`, `SM_TRACE`.
- Cada componente mantém seu prefixo: `fsm_`, `hsm_`, `ao_`.
- Headers em `include/sm/` e incluídos como `"sm/fsm.h"`, `"sm/hsm.h"`, `"sm/ao.h"`.
- Alvos de CMake com prefixo (`sm_fsm`, `sm_hsm`, `sm_ao`) e aliases `sm::fsm`, `sm::hsm`, `sm::ao`.

## Alternativas consideradas
- Manter os nomes da primeira versão: descartada — colisão provável em projeto de terceiros, e desvio não declarado do guia.
- Prefixo também nos componentes (`sm_fsm_init`): descartada — mais longo sem ganho; `fsm_`/`hsm_`/`ao_` já identificam a origem.

## Consequências
Quebra de compatibilidade com o código escrito sobre a primeira versão: includes, nomes de sinal e nome do alvo mudam. O valor numérico dos sinais reservados também mudou (quatro reservados em vez de dois); a aplicação que define seus sinais a partir de `SM_SIG_USER` não é afetada.
