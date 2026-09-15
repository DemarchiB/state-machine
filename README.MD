# state-machine

Biblioteca em C (C99) de máquinas de estado — pensada desde o início para ser simples de adicionar como submódulo em outros projetos e usar, com API pequena e sem dependências externas.

Hoje: FSM plana (`include/fsm.h`/`src/fsm.c`), pronta e testada. HSM chega como um segundo componente no mesmo repositório — decisão e motivo em [ADR-0003](docs/decisions/ADR-0003-hsm-mesmo-repositorio.md). Active Objects fica de fora daqui, em repositório próprio, por depender de fila/task/RTOS (ver [ADR-0001](docs/decisions/ADR-0001-fila-fora-da-fsm.md)).

## Pré-requisitos
- Compilador C99 (GCC ou Clang)
- CMake (`<a verificar: versão mínima ainda não confirmada por um build real>`)

## Como começar
```
git clone --recurse-submodules https://github.com/DemarchiB/state-machine.git
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

Gerador multi-config (ex.: Visual Studio)? Build e teste precisam da configuração explícita: `cmake --build build --config Debug` e `ctest --test-dir build -C Debug`.

## Documentação
- Arquitetura: [ARCHITECTURE.md](ARCHITECTURE.md)
- Instruções para agentes de IA: [AGENTS.md](AGENTS.md)
- Convenções de projeto: [docs/guide/PROJECT_GUIDE.md](docs/guide/PROJECT_GUIDE.md)
- Workflow de revisão: [docs/workflow.md](docs/workflow.md)
- Decisões: [docs/decisions/](docs/decisions/)

## Licença
`<a definir>`
