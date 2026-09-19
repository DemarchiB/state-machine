cmake_minimum_required(VERSION 3.16)
project(state-machine VERSION 1.0.0 LANGUAGES C)

# --- Opcoes ------------------------------------------------------------------
# Como projeto principal, compila testes e exemplos; como subdiretorio de
# outro projeto (add_subdirectory), so as bibliotecas.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(SM_IS_TOP_LEVEL ON)
else()
    set(SM_IS_TOP_LEVEL OFF)
endif()

option(SM_BUILD_TESTS    "Compila os testes em host (CTest)"             ${SM_IS_TOP_LEVEL})
option(SM_BUILD_EXAMPLES "Compila os exemplos"                           ${SM_IS_TOP_LEVEL})
option(SM_WARNINGS_AS_ERRORS "Avisos viram erro nos alvos deste projeto" ${SM_IS_TOP_LEVEL})
option(SM_FREERTOS_SIMULATOR "Testa a porta freertos no simulador POSIX do FreeRTOS (baixa o kernel; so Linux)" OFF)

set(SM_AO_PORT "" CACHE STRING
    "Porta do Active Object: posix, win32, freertos ou none (vazio: automatico pelo sistema)")
set(SM_AO_EVENT_SIZE_MAX 16 CACHE STRING
    "Maior evento postavel num Active Object, em bytes")
set(SM_HSM_MAX_DEPTH 8 CACHE STRING
    "Profundidade maxima da hierarquia da hsm, contando hsm_top")

if(SM_AO_PORT STREQUAL "")
    if(WIN32)
        set(SM_AO_PORT_RESOLVED win32)
    elseif(UNIX)
        set(SM_AO_PORT_RESOLVED posix)
    else()
        set(SM_AO_PORT_RESOLVED none)
    endif()
else()
    set(SM_AO_PORT_RESOLVED ${SM_AO_PORT})
endif()
if(NOT SM_AO_PORT_RESOLVED MATCHES "^(posix|win32|freertos|none)$")
    message(FATAL_ERROR "SM_AO_PORT invalido: '${SM_AO_PORT}'")
endif()
message(STATUS "state-machine: porta do Active Object = ${SM_AO_PORT_RESOLVED}")

# --- Avisos (so nos alvos deste projeto; nunca impostos a quem consome) -----
function(sm_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(SM_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wconversion -Wshadow -Wundef -Wswitch-enum)
        if(SM_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# --- Bibliotecas ------------------------------------------------------------
# sm_common: headers compartilhados (sm_event.h, sm_assert.h).
add_library(sm_common INTERFACE)
target_include_directories(sm_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(sm::common ALIAS sm_common)

add_library(sm_fsm src/fsm.c)
target_link_libraries(sm_fsm PUBLIC sm_common)
set_target_properties(sm_fsm PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
sm_enable_warnings(sm_fsm)
add_library(sm::fsm ALIAS sm_fsm)

add_library(sm_hsm src/hsm.c)
target_link_libraries(sm_hsm PUBLIC sm_common)
target_compile_definitions(sm_hsm PUBLIC HSM_MAX_DEPTH=${SM_HSM_MAX_DEPTH})
set_target_properties(sm_hsm PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
sm_enable_warnings(sm_hsm)
add_library(sm::hsm ALIAS sm_hsm)

if(NOT SM_AO_PORT_RESOLVED STREQUAL "none")
    add_library(sm_ao src/ao.c ports/${SM_AO_PORT_RESOLVED}/ao_port.c)
    target_link_libraries(sm_ao PUBLIC sm_common)
    target_include_directories(sm_ao
        PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/ports/${SM_AO_PORT_RESOLVED}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_compile_definitions(sm_ao PUBLIC AO_EVENT_SIZE_MAX=${SM_AO_EVENT_SIZE_MAX})
    set_target_properties(sm_ao PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED ON)
    if(SM_AO_PORT_RESOLVED STREQUAL "posix")
        # pthread exige as extensoes POSIX de <pthread.h>; C99 continua.
        set_target_properties(sm_ao PROPERTIES C_EXTENSIONS OFF)
        target_compile_definitions(sm_ao PRIVATE _POSIX_C_SOURCE=200809L)
        find_package(Threads REQUIRED)
        target_link_libraries(sm_ao PUBLIC Threads::Threads)
    elseif(SM_AO_PORT_RESOLVED STREQUAL "freertos")
        # O projeto que consome fornece o alvo freertos_kernel (o CMake do
        # proprio FreeRTOS-Kernel o cria, junto com freertos_config).
        target_link_libraries(sm_ao PUBLIC freertos_kernel)
    endif()
    sm_enable_warnings(sm_ao)
    add_library(sm::ao ALIAS sm_ao)
endif()

# --- Exemplos e testes -------------------------------------------------------
if(SM_BUILD_EXAMPLES)
    add_subdirectory(examples/fsm_catraca)
    add_subdirectory(examples/hsm_catraca)
    if(SM_AO_PORT_RESOLVED MATCHES "^(posix|win32)$")
        add_subdirectory(examples/ao_catraca)
    endif()
endif()

if(SM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
