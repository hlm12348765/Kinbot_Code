# decision_orchestration — 决策编排

> OODA 角色：Decide 阶段的核心——状态机 + 行为树 + OODA Scale Scheduler
> 域团队：S4 世界状态与决策

## 职责

- 分层状态机：5 顶层状态 + 8 业务状态 + 4 约束子状态
- 行为树叶节点执行
- OODA Scale Scheduler：R1-R4 子环的动态调度（6 输入 × 7 切换规则）
- ActionProposal 生成

## 关键接口

- 消费：DecisionContextSnapshot, HealthEventCandidate
- 产出：ActionProposal

## 架构文档

- [系统决策状态机](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/06_decision_state_machine.md)
- [多尺度动态 OODA 架构基线](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/03_multi_scale_dynamic_ooda_architecture_baseline.md)
