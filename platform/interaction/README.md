# multimodal_interaction — 多模态交互

> OODA 角色：Act 阶段的交互执行，Observe 阶段的语音输入
> 域团队：S3 多模态交互与陪伴

## 职责

- 对话管理与 NLU
- TTS 与人设语音
- 屏幕显示与灯光控制
- 主动交互触发（5 类）
- 夜间静默规则
- 中断与恢复

## 关键接口

- 消费：ApprovalDecision, WorldStateSnapshot, HealthEventCandidate
- 产出：WorldEvent (VOICE_COMMAND_RECEIVED)

## 架构文档

- [陪伴交互策略](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/08_companion_interaction_strategy.md)
- [模块分层与模块边界](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/04_module_layers_and_boundaries.md)
