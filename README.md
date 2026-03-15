# Kinbot Code

家庭室内智能移动交互机器人系统 - 代码仓库

## 架构文档

本仓库为代码实现仓库。系统架构设计文档在独立仓库维护：

**[Kinbot_OODA 架构文档仓库](https://github.com/hlm12348765/Kinbot_OODA)**

推荐阅读顺序：
1. 总体架构 (`docs/02_p1_architecture/01_overall_architecture.md`)
2. 多尺度动态 OODA 架构基线 (`docs/02_p1_architecture/03_multi_scale_dynamic_ooda_architecture_baseline.md`)
3. 模块分层与模块边界 (`docs/02_p1_architecture/04_module_layers_and_boundaries.md`)
4. 世界状态与核心实体结构 (`docs/02_p1_architecture/05_world_state_schema.md`)
5. 安全/合规/授权接口 (`docs/02_p1_architecture/07_safety_compliance_authorization_api.md`)

## 仓库结构

```
Kinbot_Code/
├── platform/           端侧运行栈（8 个模块）
│   ├── runtime/        事件总线、状态总线、进程管理、资源调度
│   ├── mobility/       建图、定位、路径规划、底盘控制
│   ├── perception/     人体检测、姿态/跌倒、语音、穿戴接入
│   ├── interaction/    对话、TTS、屏幕、灯光、手势
│   ├── world_state/    三层状态、长期记忆
│   ├── decision/       状态机、行为树、OODA Scale Scheduler
│   ├── safety/         审批门控、审计
│   └── observability/  日志、指标、追踪、数据治理
├── companion/          伴生系统
│   ├── app/            家属 App (Flutter)
│   ├── cloud/          云服务 (Go)
│   └── ops/            后台运营坐席 (Web)
├── chassis/            R1 反射环底层安全栈 (RTOS, C/C++)
├── contracts/          跨模块接口定义 (Protobuf IDL) — 单一真相源
├── tools/              构建、部署、烧录、OTA 工具链
├── tests/              系统级集成测试、场景回放、故障注入
└── docs/               链接回架构文档仓库
```

## 技术栈

| 层级 | 语言/框架 | 运行环境 |
|------|-----------|----------|
| R1 底盘安全栈 | C / C++17 | RTOS (FreeRTOS / Zephyr) |
| R2-R4 端侧应用 | C++17 / Python 3.11+ | Linux (RK3588 / Digua S100 Pro) |
| 模型推理 | ONNX Runtime / TensorRT | NPU/GPU on SoC |
| 接口定义 | Protobuf v3 | 跨端 |
| 家属 App | Flutter | iOS / Android |
| 云服务 | Go | K8s |

## 构建

```bash
# Host 编译
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# ARM64 交叉编译
cmake -B build-arm64 -DCMAKE_TOOLCHAIN_FILE=tools/cross-compile/aarch64-linux.cmake
cmake --build build-arm64 -j$(nproc)
```

## 团队与模块映射

| 域团队 | 代码模块 |
|--------|---------|
| S1 本体平台与运动 | `chassis/` + `platform/runtime/` + `platform/mobility/` |
| S2 人体感知与健康 | `platform/perception/` |
| S3 多模态交互与陪伴 | `platform/interaction/` |
| S4 世界状态与决策 | `platform/world_state/` + `platform/decision/` |
| S5 安全合规授权 | `platform/safety/` |
| S6 伴生系统与服务 | `companion/` |
| S7 治理观测与数据 | `platform/observability/` |
