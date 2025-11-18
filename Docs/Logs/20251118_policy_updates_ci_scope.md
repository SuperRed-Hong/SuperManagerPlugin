# policy updates ci scope

## 问题描述
- 具体问题：需要在规范中记录下一步建议、扩展 UE 源码核对规则，并缩小 CI 扫描范围
- 影响范围：所有 SuperManager 提交流程与自动化检查

## 解决方案
- 技术方案：更新 AGENTS/AGENTS_summary、限制 CI 扫描路径、扩展日志模板支持下一步建议
- 修改的文件：AGENTS.md,Docs/AGENTS_summary.md,Scripts/ci/check_agent_guidelines.py,Scripts/tools/create_log_entry.py,.github/pull_request_template.md

## 实施步骤
1. 在 AGENTS 中新增日志模板的下一步建议章节并强调同步要求
2. 将 Docs/AGENTS_summary 的迁移注意事项改为整套 UE 源码核对规则
3. 限制 check_agent_guidelines.py 仅扫描 Plugins/SuperManager/Source/SuperManager 并新增 PR 模板
4. 扩展 create_log_entry.py 支持 --next-step 并更新模板

## 测试验证
- python3 Scripts/ci/check_agent_guidelines.py
- python3 Scripts/tools/create_log_entry.py --help
- 测试结果：CI/工具脚本运行成功

## 注意事项
- 潜在风险：CI 目前只覆盖 SuperManager，其他模块如需纳入需人工更新
- 后续优化：后续可在 README/Onboarding 中引用新的 PR 模板要求

## 问题提醒
- 用户此前只关注 Slate 5.6 的 API 变更而忽视其他引擎模块，必须改为校对整个 UE 源码
- 若不在 README/指南说明 CI/PR 模板要求，新成员仍会漏跑流程

## 下一步建议
- 在 README/贡献指南中说明 CI/PR 模板流程，避免新人遗漏
- 考虑把日志脚本整合到 git hook，强制填入下一步建议
