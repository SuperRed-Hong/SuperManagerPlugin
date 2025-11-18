# document palette usage

## 问题描述
- 具体问题：需要在代码中解释 Primary/Accent 色板用途
- 影响范围：SuperManager 全部 Slate

## 解决方案
- 技术方案：为 FSuperManagerStyle 静态成员添加 Doxygen 注释说明适用场景
- 修改的文件：Plugins/SuperManager/Source/SuperManager/Public/CustomStyle/SuperManagerStyle.h

## 实施步骤
1. 在头文件 static 成员上补充用途注释

## 测试验证
- python3 Scripts/ci/check_agent_guidelines.py
- 测试结果：脚本通过

## 注意事项
- 潜在风险：若未来新增色板需保持注释同步
- 后续优化：可考虑将这些注释同步到 README 以便查阅

## 问题提醒
- 尽量在代码中直接说明用途，减少额外文档同步成本

## 下一步建议
- 后续若新建色值/Brush，记得按相同模板补注释
