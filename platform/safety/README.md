# safety_compliance_authorization — 安全合规授权

> OODA 角色：Decide→Act 之间的前置门控
> 域团队：S5 安全合规授权

## 职责

- evaluate_action() 统一审批接口
- 11 类受控动作审批
- 6 种决策结果：approved / rejected / downgraded / confirmation_required / manual_service_required / fault_protection_required
- A1-A7 高风险异常上下文
- F1-F7 关键安全故障硬中断
- 审计日志

## 关键接口

- 消费：ActionProposal, PolicyContext
- 产出：ApprovalDecision, ConfirmationRequest

## 架构文档

- [安全/合规/授权接口](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/07_safety_compliance_authorization_api.md)
- [安全风险矩阵](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/09_safety_risk_matrix.md)
