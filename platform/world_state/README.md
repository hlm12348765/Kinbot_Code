# world_state_memory — 世界状态与记忆

> OODA 角色：Orient 阶段的核心——统一状态平面
> 域团队：S4 世界状态与决策

## 职责

- 三层状态管理：snapshot / session / persistent
- 9 类一级实体 CRUD：Person, RoleBinding, Household, Place, Object, HealthProfile, MedicationAsset, Task, RiskEvent
- 长期记忆治理（5 类记忆）
- 事实与判断的分层管理
- 隐私分级（4 级）

## 关键接口

- 产出：WorldStateSnapshot, DecisionContextSnapshot
- 消费：WorldEvent (all types)

## 架构文档

- [世界状态与核心实体结构](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/05_world_state_schema.md)
- [模块分层与模块边界](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/04_module_layers_and_boundaries.md)
