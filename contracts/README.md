# contracts/ — 跨模块接口定义

> 接口单一真相源。所有模块间通信通过此处定义的 Protobuf IDL。

## Proto 文件结构

```
kinbot/
├── common/v1/common.proto              共享类型：EntityMeta, PrivacyLevel, RiskLevel, Role
├── safety/v1/action_proposal.proto     ActionType(11类), ActionProposal
├── safety/v1/approval_decision.proto   ApprovalResult(6种), ApprovalDecision, ConfirmationRequest
├── world_state/v1/world_state.proto    9 类一级实体 + DecisionContextSnapshot
├── health/v1/health_event.proto        HealthEventCandidate(7级管线), VitalSignalReading
├── event_bus/v1/robot_event.proto      WorldEvent(22种), RobotEventUplink
└── companion/v1/companion.proto        FamilyNotification, FamilyConfirmationResponse
```

## 代码生成

CI 自动生成 C++ / Python / Go / Dart 绑定到 `generated/` 目录（已 .gitignore）。

## 接口变更规则

1. 新增字段必须向后兼容（新字段用新编号，不复用已删除编号）
2. Breaking change 须通过架构组审批
3. 每次变更须更新对应的架构文档

## 架构文档

- [世界状态与核心实体结构](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/05_world_state_schema.md)
- [安全/合规/授权接口](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/07_safety_compliance_authorization_api.md)
- [健康事件管线](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/10_health_event_pipeline_and_escalation.md)
