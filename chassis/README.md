# R1 底盘安全栈

> OODA 角色：R1 反射环——硬实时安全闭环
> 域团队：S1 本体平台与运动

## 职责

- ms 级急停响应
- 障碍物避让
- 防夹手保护
- 轮速控制与里程计
- 硬件 watchdog

## 运行环境

- RTOS (FreeRTOS / Zephyr) 或裸机 MCU
- 与 Linux 用户态通过硬件抽象层通信
- 独立于 platform/，保证实时性

## 架构文档

- [多尺度动态 OODA 架构基线](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/03_multi_scale_dynamic_ooda_architecture_baseline.md)
