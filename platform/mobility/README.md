# mobility_navigation — 导航与运动

> OODA 角色：Act 阶段执行移动指令，Orient 阶段提供位置感知
> 域团队：S1 本体平台与运动

## 职责

- 全屋建图与重定位
- 路径规划与避障
- 7 种导航技能：go_to_room, search_person, approach_person, follow_person 等
- 社交移动策略（靠近前语音预告、安全距离）

## 关键接口

- 消费：ActionProposal (NAVIGATE_TO_PERSON), WorldStateSnapshot (Place, Person)
- 产出：WorldEvent (ROBOT_NEAR_ELDER, ROBOT_DELIVERY_COMPLETED)

## 架构文档

- [模块分层与模块边界](https://github.com/hlm12348765/Kinbot_OODA/blob/main/docs/02_p1_architecture/04_module_layers_and_boundaries.md)
