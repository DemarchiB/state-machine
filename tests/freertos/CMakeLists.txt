# Teste da porta freertos no simulador POSIX do FreeRTOS-Kernel (Linux).
# Dependencia so de teste, com versao fixada: tag V11.1.0, verificada pelo
# commit.
include(FetchContent)

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

set(FREERTOS_PORT GCC_POSIX CACHE STRING "" FORCE)
set(FREERTOS_HEAP 3 CACHE STRING "" FORCE) # malloc do host; nao usado por sm_ao

FetchContent_Declare(freertos_kernel
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
    GIT_TAG        dbf70559b27d39c1fdb68dfb9a32140b6a6777a0 # V11.1.0
)
FetchContent_MakeAvailable(freertos_kernel)

add_executable(test_ao_freertos test_ao_freertos.c ${SM_EXAMPLES_DIR}/fsm_catraca/catraca.c)
target_include_directories(test_ao_freertos PRIVATE
    ${SM_EXAMPLES_DIR}/fsm_catraca ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_ao_freertos PRIVATE sm::ao sm::fsm freertos_kernel)
target_compile_definitions(test_ao_freertos PRIVATE SM_TRACE=1)
sm_enable_warnings(test_ao_freertos)
add_test(NAME test_ao_freertos COMMAND test_ao_freertos)
set_tests_properties(test_ao_freertos PROPERTIES TIMEOUT 20)
