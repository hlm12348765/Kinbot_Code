add_library(kinbot_world_state STATIC
    src/world_state.cpp
    src/world_state_manager.cpp
)
target_include_directories(kinbot_world_state PUBLIC include)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
