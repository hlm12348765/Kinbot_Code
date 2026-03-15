# World State 模块实现计划

## 目标
实现 `platform/world_state` 模块的核心逻辑，使其能够：
- 接收 WorldEvent 事件，更新内部状态（9类一级实体）
- 生成 WorldStateSnapshot / DecisionContextSnapshot 供 decision 模块消费
- 提供本地可运行的 GoogleTest 测试用例，验证"输入事件 → 状态更新/跳转"

## 设计决策

### 为什么不直接依赖 Protobuf 生成代码？
Proto 编译需要安装 protoc + libprotobuf-dev，增加本地构建门槛。Phase 0 先用**纯 C++ 结构体**（镜像 proto 定义）实现核心逻辑，后续再替换为 proto 生成代码。这样测试可以零外部依赖运行。

### 核心类设计
```
WorldState          — 持有所有实体的内存容器，提供 CRUD
WorldStateManager   — 接收 WorldEvent，调用 WorldState 做增删改查，生成 Snapshot
```

## 实现步骤

### Step 1: 定义纯 C++ 数据结构（镜像 proto）
- `platform/world_state/include/kinbot/world_state/types.h`
  - 9 类实体结构体：Person, RoleBinding, Household, Place, Object, HealthProfile, MedicationAsset, Task, RiskEvent
  - 运行时状态：BatteryState, NetworkState, WearableFreshnessState
  - WorldEvent 结构体（含 WorldEventType 枚举）
  - DecisionContextSnapshot, WorldStateSnapshot

### Step 2: 实现 WorldState 容器
- `platform/world_state/include/kinbot/world_state/world_state.h`
- `platform/world_state/src/world_state.cpp`
- 功能：
  - 各实体的 add / get / update / remove
  - 按条件查询（如 active persons、active tasks）
  - 生成 DecisionContextSnapshot
  - 版本号自增

### Step 3: 实现 WorldStateManager（事件驱动入口）
- `platform/world_state/include/kinbot/world_state/world_state_manager.h`
- `platform/world_state/src/world_state_manager.cpp`
- 功能：
  - `handle_event(WorldEvent)` — 事件分发，根据 event_type 调用对应 handler
  - 事件处理映射（22 种 WorldEventType → 对应状态更新逻辑）
  - Phase 0 先实现以下高优先级事件：
    1. PERSON_DETECTED → 添加/更新 Person（anonymous）
    2. PERSON_IDENTIFIED → 更新 Person identity_status
    3. PERSON_LOST → 从 active 列表移除
    4. FALL_SUSPECTED → 创建 RiskEvent
    5. TASK_CREATED → 添加 Task
    6. TASK_STATE_CHANGED → 更新 Task status
    7. MEDICATION_DUE → 创建送药 Task + 设置 medication_urgency
    8. ABNORMAL_VITAL_RECEIVED → 创建 RiskEvent + 更新 care_priority
  - `get_snapshot()` → 返回当前 WorldStateSnapshot

### Step 4: 更新 CMakeLists.txt
- 将 world_state 从 INTERFACE 改为 STATIC 库
- 添加 GoogleTest 依赖（FetchContent）
- 添加测试目标

### Step 5: 编写测试用例
- `platform/world_state/tests/world_state_test.cpp`
- `platform/world_state/tests/world_state_manager_test.cpp`
- 测试场景：
  1. **人物检测流程**：PERSON_DETECTED → PERSON_IDENTIFIED → 验证 Person 实体状态
  2. **人物丢失**：PERSON_LOST → 验证从 active 列表移除
  3. **跌倒风险**：FALL_SUSPECTED → 验证 RiskEvent 创建 + snapshot 中 health_alerts 更新
  4. **任务生命周期**：TASK_CREATED → TASK_STATE_CHANGED(executing) → TASK_STATE_CHANGED(completed)
  5. **送药场景**：MEDICATION_DUE → 验证 Task 创建 + medication_urgency = true
  6. **异常生命体征**：ABNORMAL_VITAL_RECEIVED → 验证 RiskEvent + care_priority 升级
  7. **快照一致性**：多个事件后，验证 DecisionContextSnapshot 内容正确
  8. **幂等性**：重复事件不会创建重复实体

## 文件清单

```
platform/world_state/
├── CMakeLists.txt                          (修改)
├── include/kinbot/world_state/
│   ├── types.h                             (新增)
│   ├── world_state.h                       (新增)
│   └── world_state_manager.h               (新增)
├── src/
│   ├── world_state.cpp                     (新增)
│   └── world_state_manager.cpp             (新增)
└── tests/
    ├── CMakeLists.txt                      (新增)
    ├── world_state_test.cpp                (新增)
    └── world_state_manager_test.cpp        (新增)
```

## 本地运行方式

```bash
mkdir -p build && cd build
cmake .. -DBUILD_TESTING=ON
make -j$(nproc)
ctest --output-on-failure
```
