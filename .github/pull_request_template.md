## 概述
- 变更目的：
- 方案摘要：
- 相关 Issue/任务链接：

## 测试清单
- [ ] 本地执行 `python3 Scripts/ci/check_agent_guidelines.py`
- [ ] 若修改 C++/Slate，提供 `UnrealBuildTool`/编辑器编译或运行日志
- [ ] 附上 10 秒内可复现的验证步骤或截图

## 规范检查
- [ ] 所有公共类/函数/属性均补充 Doxygen 注释
- [ ] 若新增/修改 Slate 交互，已核对 `Docs/UESource` (UE5.6) 中的头文件签名（如 `SHeaderRow`, `UI_COMMAND`, `FSlateRoundedBoxBrush` 等）
- [ ] 若修改 `.Build.cs` 依赖，提醒评审“需重新生成项目文件并编译”
- [ ] 已在 `Docs/Logs` 创建对应日志并填写“问题-方案-验证-风险”
- [ ] 说明是否改动第三方/自动生成内容（如 RiderLink），若有需同步更新白名单策略

## 其他备注
- 风险或后续 TODO：
- 需关注的回滚点/监控指标：
