# agent guardrail helpers

## 问题描述
- 具体问题：缺少速查文档与自动化工具
- 影响范围：所有参与 ExtendEditor 项目的协作者

## 解决方案
- 技术方案：新增速查表、日志模板脚本与 CI 检查脚本
- 修改的文件：Docs/AGENTS_summary.md, Scripts/tools/create_log_entry.py, Scripts/ci/check_agent_guidelines.py

## 实施步骤
1. 整理 AGENTS 要点并写入 Docs/AGENTS_summary.md
2. 实现 Docs/Logs 生成脚本 Scripts/tools/create_log_entry.py
3. 实现公共接口注释与 Docs/UESource 只读检查脚本

## 测试验证
- python3 Scripts/tools/create_log_entry.py --title demo --dry-run
- python3 Scripts/ci/check_agent_guidelines.py --help
- 测试结果：脚本运行输出与帮助信息符合预期

## 注意事项
- 潜在风险：CI 检查目前只针对宏注释，可能产生漏报
- 后续优化：可继续扩展对命名规范和日志生成的自动验证
