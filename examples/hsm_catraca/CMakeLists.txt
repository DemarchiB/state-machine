add_executable(example_hsm_catraca catraca_hsm.c main.c)
target_link_libraries(example_hsm_catraca PRIVATE sm::hsm)
sm_enable_warnings(example_hsm_catraca)
