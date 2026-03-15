# human_health_sensing — 人体感知与健康

> OODA 角色：Observe 阶段的核心感知模块
> 域团队：S2 人体感知与健康

## 职责

- 人体检测与身份识别
- 姿态估计与跌倒检测
- 穿戴设备数据接入（BLE：心率、血氧、血压）
- 健康事件 7 级管线的前 2 级（信号采集 + 融合）
- VLM 视觉理解

## 关键接口

- 产出：HealthEventCandidate, VitalSignalReading, WorldEvent (PERSON_DETECTED, FALL_SUSPECTED, ...)
- 消费：WorldStateSnapshot (Person, HealthProfile)

## 架构文档

- [健康事件管线](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/10_health_event_pipeline_and_escalation.md)
- [模块分层与模块边界](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/04_module_layers_and_boundaries.md)
