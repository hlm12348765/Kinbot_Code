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

## 当前落地范围

- `WorldStateStore`：纯内存状态存储，基于 `WorldEvent` 做最小 reducer
- 已实现事件：`PERSON_IDENTIFIED`、`FALL_SUSPECTED`、`MEDICATION_DUE`、`TASK_STATE_CHANGED`、`MANUAL_SERVICE_*`
- 本地 CLI：`kinbot_world_state_cli`
- 本地 SIL 测试：`kinbot_world_state_sil_test`

## 本地运行

从仓库根目录构建：

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j

# 跑最小 world_state 场景测试
ctest --test-dir build --output-on-failure -R kinbot_world_state_sil_test

# 手动输入事件，观察 world_state 更新
./build/platform/world_state/kinbot_world_state_cli \
  PERSON_IDENTIFIED person_id=elder_1 display_name=Grandma age_group=elder \
  --then FALL_SUSPECTED person_id=elder_1 risk_event_id=fall_001 severity=CRITICAL
```

从 `platform/world_state` 子目录单独构建：

```bash
cd platform/world_state
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/kinbot_world_state_cli PERSON_IDENTIFIED person_id=elder_1
```

## 架构文档

- [世界状态与核心实体结构](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/05_world_state_schema.md)
- [模块分层与模块边界](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/04_module_layers_and_boundaries.md)
