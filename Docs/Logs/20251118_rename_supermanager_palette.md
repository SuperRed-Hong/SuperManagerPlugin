# rename supermanager palette

## 问题描述
- 具体问题：需要把 LockedActors 命名改成通用 UI palette
- 影响范围：SuperManager 全部 Slate

## 解决方案
- 技术方案：FSuperManagerStyle 静态成员改为 Primary/Accent 命名并更新引用
- 修改的文件：Plugins/SuperManager/Source/SuperManager/Public/CustomStyle/SuperManagerStyle.h,Plugins/SuperManager/Source/SuperManager/Private/CustomStyle/SuperManagerStyle.cpp,Plugins/SuperManager/Source/SuperManager/Private/SlateWidgets/LockedActorsListWidget.cpp,Plugins/SuperManager/Source/SuperManager/Private/SlateWidgets/LockedActorsListRow.cpp

## 实施步骤
1. 重命名静态颜色/Brush 成 Primary/Accent
2. 更新 Widget/Row 引用并保持按钮白色

## 测试验证
- python3 Scripts/ci/check_agent_guidelines.py
- 测试结果：脚本通过

## 注意事项
- 潜在风险：若其他 Slate 仍引用旧名需同步更新
- 后续优化：可在文档注明 Primary/Accent 配色含义

## 问题提醒
- UI 配色已升级为通用命名，团队别再写 LockedActors 前缀或散落颜色

## 下一步建议
- 在 README/样式指南写清 Primary/Accent 颜色含义与使用场景
