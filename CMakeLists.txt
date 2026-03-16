cmake_minimum_required(VERSION 3.22)
project(kinbot VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

# Protobuf 接口定义
add_subdirectory(contracts)

# 端侧运行栈
add_subdirectory(platform)

# R1 底盘安全栈
add_subdirectory(chassis)

# 测试
add_subdirectory(tests)
