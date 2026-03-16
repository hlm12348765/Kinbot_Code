add_executable(kinbot_world_state_sil_test
  world_state_flow_test.cpp
)

target_link_libraries(kinbot_world_state_sil_test PRIVATE kinbot_world_state)

add_test(
  NAME kinbot_world_state_sil_test
  COMMAND kinbot_world_state_sil_test
)
