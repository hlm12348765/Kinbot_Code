cmake_minimum_required(VERSION 3.22)
project(kinbot VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Protobuf 接口定义（需要安装 protobuf，Phase 0 可关闭）
option(KINBOT_BUILD_CONTRACTS "Build protobuf contracts" OFF)
if(KINBOT_BUILD_CONTRACTS)
    add_subdirectory(contracts)
endif()

# 端侧运行栈
add_subdirectory(platform)

# R1 底盘安全栈
add_subdirectory(chassis)

# 测试
enable_testing()
