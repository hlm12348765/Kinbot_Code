# platform_runtime — 端侧运行栈基础

> OODA 角色：所有 R2-R4 子环的运行基础设施
> 域团队：S1 本体平台与运动

## 职责

- 事件总线：模块间异步消息分发
- 状态总线：World State 读写通道
- 进程管理：模块启停、心跳、故障恢复
- 资源调度：CPU/NPU/内存分配

## 不负责

- 业务逻辑决策（由 decision 模块负责）
- 感知算法（由 perception 模块负责）

## 关键接口

- 产出：WorldEvent 分发、WorldStateSnapshot 读写通道
- 消费：各模块注册的事件订阅

## 架构文档

- [模块分层与模块边界](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/04_module_layers_and_boundaries.md)
- [总体架构](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/01_overall_architecture.md)
