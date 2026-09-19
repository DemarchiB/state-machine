set(SM_EXAMPLES_DIR ${PROJECT_SOURCE_DIR}/examples)

# fsm, sobre o exemplo da catraca. SM_TRACE = 1 so neste alvo: a
# biblioteca e o exemplo continuam sem instrumentacao.
add_executable(test_fsm test_fsm.c ${SM_EXAMPLES_DIR}/fsm_catraca/catraca.c)
target_include_directories(test_fsm PRIVATE ${SM_EXAMPLES_DIR}/fsm_catraca)
target_link_libraries(test_fsm PRIVATE sm::fsm)
target_compile_definitions(test_fsm PRIVATE SM_TRACE=1)
sm_enable_warnings(test_fsm)
add_test(NAME test_fsm COMMAND test_fsm)

# hsm: algoritmo, com hierarquia sintetica. O trace e contado, entao o
# hook precisa existir tambem dentro da biblioteca: um alvo proprio da hsm
# com SM_TRACE = 1.
add_library(sm_hsm_trace STATIC ${PROJECT_SOURCE_DIR}/src/hsm.c)
target_link_libraries(sm_hsm_trace PUBLIC sm_common)
target_compile_definitions(sm_hsm_trace PUBLIC SM_TRACE=1 HSM_MAX_DEPTH=${SM_HSM_MAX_DEPTH})
sm_enable_warnings(sm_hsm_trace)

add_executable(test_hsm test_hsm.c)
target_link_libraries(test_hsm PRIVATE sm_hsm_trace)
sm_enable_warnings(test_hsm)
add_test(NAME test_hsm COMMAND test_hsm)

# hsm: exemplo da catraca hierarquica.
add_executable(test_hsm_catraca test_hsm_catraca.c ${SM_EXAMPLES_DIR}/hsm_catraca/catraca_hsm.c)
target_include_directories(test_hsm_catraca PRIVATE ${SM_EXAMPLES_DIR}/hsm_catraca)
target_link_libraries(test_hsm_catraca PRIVATE sm::hsm)
sm_enable_warnings(test_hsm_catraca)
add_test(NAME test_hsm_catraca COMMAND test_hsm_catraca)

# ao: portas de host (threads reais).
if(SM_AO_PORT_RESOLVED MATCHES "^(posix|win32)$")
    add_executable(test_ao test_ao.c ${SM_EXAMPLES_DIR}/fsm_catraca/catraca.c)
    target_include_directories(test_ao PRIVATE ${SM_EXAMPLES_DIR}/fsm_catraca)
    target_link_libraries(test_ao PRIVATE sm::ao sm::fsm)
    target_compile_definitions(test_ao PRIVATE SM_TRACE=1)
    sm_enable_warnings(test_ao)
    add_test(NAME test_ao COMMAND test_ao)
    set_tests_properties(test_ao PROPERTIES TIMEOUT 10)
endif()

# ao: porta freertos, sobre o simulador POSIX do proprio FreeRTOS.
if(SM_AO_PORT_RESOLVED STREQUAL "freertos" AND SM_FREERTOS_SIMULATOR)
    add_subdirectory(freertos)
endif()
