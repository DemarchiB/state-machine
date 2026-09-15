# fsm

Biblioteca em C (C99) de máquinas de estado finitas — pensada desde o início para ser simples de adicionar como submódulo em outros projetos e usar, com API pequena e sem dependências externas. Base para bibliotecas de HSM e Active Objects futuras.

## Pré-requisitos
- Compilador C99 (GCC ou Clang)
- CMake (`<a verificar: versão mínima ainda não confirmada por um build real>`)

## Como começar
```
git clone --recurse-submodules https://github.com/DemarchiB/fsm.git
cmake -S . -B build
cmake --build build
```

Testes em host (`ctest`) chegam junto com a primeira implementação — por enquanto o build só configura a interface pública em `include/fsm.h`, sem nada pra compilar.

## Documentação
- Arquitetura: [ARCHITECTURE.md](ARCHITECTURE.md)
- Instruções para agentes de IA: [AGENTS.md](AGENTS.md)
- Convenções de projeto: [docs/guide/PROJECT_GUIDE.md](docs/guide/PROJECT_GUIDE.md)
- Workflow de revisão: [docs/workflow.md](docs/workflow.md)

## Licença
`<a definir>`
