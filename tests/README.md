# 系统级集成测试

## 子目录

- `sil/` — 软件在环 (Software-in-the-Loop) 仿真
- `scenarios/` — 场景回放数据
- `fixtures/` — Mock 传感器数据源

## 当前可运行用例

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure -R kinbot_world_state_sil_test
```

`kinbot_world_state_sil_test` 会顺序验证：

- `PERSON_IDENTIFIED` 写入 `active_persons`
- `FALL_SUSPECTED` 生成 `health_alerts` 并切换 `home_mode -> ANOMALY_ACTIVE`
- `MANUAL_SERVICE_*` 触发 `manual_service_state` 跳转
- `MEDICATION_DUE` 与 `TASK_STATE_CHANGED` 驱动任务状态和 `medication_urgency` 更新
