# R1 底盘安全栈
# Phase 0: host 编译做接口验证，后续切换到 RTOS 交叉编译
add_library(kinbot_chassis INTERFACE)
target_include_directories(kinbot_chassis INTERFACE include)
