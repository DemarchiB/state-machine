add_executable(example_fsm_catraca catraca.c main.c)
target_link_libraries(example_fsm_catraca PRIVATE sm::fsm)
sm_enable_warnings(example_fsm_catraca)
