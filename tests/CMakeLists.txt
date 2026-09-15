add_executable(test_catraca
    test_catraca.c
    ${CMAKE_SOURCE_DIR}/examples/catraca/catraca.c
)
target_link_libraries(test_catraca PRIVATE fsm)
target_include_directories(test_catraca PRIVATE ${CMAKE_SOURCE_DIR}/examples/catraca)

# Parte 4 do exercicio: liga o trace so neste alvo de teste -- a lib e o
# exemplo continuam sem instrumentacao por padrao.
target_compile_definitions(test_catraca PRIVATE FSM_TRACE)

add_test(NAME test_catraca COMMAND test_catraca)
