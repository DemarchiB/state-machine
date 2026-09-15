cmake_minimum_required(VERSION 3.16)
project(state-machine C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(fsm src/fsm.c)
target_include_directories(fsm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(examples/catraca)

enable_testing()
add_subdirectory(tests)

# hsm (target) entra aqui quando include/hsm.h + src/hsm.c existirem de
# verdade -- ver docs/decisions/ADR-0003-hsm-mesmo-repositorio.md.
