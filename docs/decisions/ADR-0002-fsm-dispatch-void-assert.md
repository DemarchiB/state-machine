# ADR-0002: `fsm_init`/`fsm_dispatch` retornam `void`; contrato validado por assert

- **Status:** aceito
- **Data:** 2026-09-15
- **Substitui:** —

## Contexto
A interface pública decidida na Fase 2 (`include/fsm.h`) tinha `fsm_init`/`fsm_dispatch` retornando `fsm_err_t`, seguindo à risca a convenção de `docs/guide/templates/modulo-c.md` ("o construtor valida e retorna erro"). Mas `fsm_dispatch` não é um construtor — é chamado a cada evento, potencialmente centenas de vezes por segundo, e os únicos parâmetros possíveis de invalidar são ponteiros nulos, sem faixa de valor real pra checar. Um código de erro que a maioria dos chamadores nunca lê não protege nada; só ocupa espaço e um `if` a mais no caminho mais quente da biblioteca.

## Decisão
`fsm_init` e `fsm_dispatch` retornam `void`. `me`, `estado_inicial` e `e` não podem ser `NULL` — isso é pré-condição de contrato (bug de quem chama), não condição de erro recuperável em runtime. A violação é verificada por uma macro de assert (`FSM_ASSERT`, hoje um wrapper fino sobre `assert()` de `<assert.h>`), não por valor de retorno.

## Alternativas consideradas
- `fsm_err_t` como retorno (decisão original da Fase 2): descartada — o padrão de "construtor valida e retorna erro" do guia é pensado pra parâmetros com faixa de valor real (ex.: corrente em mA), não pra ponteiros que ou existem ou são bug. Manter o retorno de erro obriga a checar (ou ignorar, silenciosamente) um código em todo `fsm_dispatch`.
- Nenhuma validação (confiar cegamente no chamador): descartada — sem `assert`, um `NULL` vira undefined behavior silencioso, o pior dos dois casos.

## Consequências
Em build de teste/debug (`assert` ativo), um `NULL` interrompe imediatamente, apontando o bug. Em build de release com `NDEBUG`, a checagem some — mesmo modelo do QP/C, referência de arquitetura desta lib. Transformar isso num comportamento seguro em vez de simplesmente sumir em release (um handler de assert customizado que força estado seguro) fica em aberto para a Fase 6 (safety) — não decidido agora.
