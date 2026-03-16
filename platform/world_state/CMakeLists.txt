cmake_minimum_required(VERSION 3.22)

set(KINBOT_WORLD_STATE_TOPLEVEL OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(KINBOT_WORLD_STATE_TOPLEVEL ON)
  project(kinbot_world_state VERSION 0.1.0 LANGUAGES CXX)

  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  enable_testing()
endif()

if(NOT TARGET kinbot_contracts)
  get_filename_component(KINBOT_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
  add_subdirectory("${KINBOT_REPO_ROOT}/contracts" "${CMAKE_CURRENT_BINARY_DIR}/contracts")
endif()

# CMake 4.2 + Unix Makefiles 在当前子工程布局下不会稳定预建顶层 target 的对象目录。
# 显式创建它们，避免 clang 在写 .o 文件时因目录缺失失败。
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/kinbot_world_state.dir/src")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/kinbot_world_state_cli.dir/src")

add_library(kinbot_world_state
  src/world_state_store.cpp
)

target_include_directories(kinbot_world_state
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/contracts
)
target_link_libraries(kinbot_world_state PUBLIC kinbot_contracts)

add_executable(kinbot_world_state_cli
  src/world_state_cli.cpp
)
target_link_libraries(kinbot_world_state_cli PRIVATE kinbot_world_state)

if(KINBOT_WORLD_STATE_TOPLEVEL)
  add_subdirectory("${KINBOT_REPO_ROOT}/tests/sil" "${CMAKE_CURRENT_BINARY_DIR}/tests/sil")
endif()
