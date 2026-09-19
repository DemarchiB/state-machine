# So nas portas de host: usa ao_join, que a porta freertos nao tem.
add_executable(example_ao_catraca main.c ../fsm_catraca/catraca.c)
target_include_directories(example_ao_catraca PRIVATE ../fsm_catraca)
target_link_libraries(example_ao_catraca PRIVATE sm::ao sm::fsm)
sm_enable_warnings(example_ao_catraca)
