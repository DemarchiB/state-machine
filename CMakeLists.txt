cmake_minimum_required(VERSION 3.16)
project(fsm C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# So a interface publica existe (include/fsm.h). Sem fsm.c ainda, por isso
# INTERFACE -- nao ha nada pra compilar de verdade ainda.
add_library(fsm INTERFACE)
target_include_directories(fsm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# src/, tests/ (CTest) e examples/ entram quando fsm.c e os primeiros
# testes existirem.
