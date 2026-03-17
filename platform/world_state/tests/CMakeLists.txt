include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(world_state_test
    world_state_test.cpp
)
target_link_libraries(world_state_test PRIVATE
    kinbot_world_state
    GTest::gtest_main
)

add_executable(world_state_manager_test
    world_state_manager_test.cpp
)
target_link_libraries(world_state_manager_test PRIVATE
    kinbot_world_state
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(world_state_test)
gtest_discover_tests(world_state_manager_test)
