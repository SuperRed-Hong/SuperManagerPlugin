# centralize locked actors style

## 问题描述
- 具体问题：LockedActorsListWidget/Row 各自维护颜色与 Brush 常量，导致风格分散
- 影响范围：SuperManager 插件 Locked Actors 面板的样式维护

## 解决方案
- 技术方案：将所有颜色与 RoundedBoxBrush 挪到 FSuperManagerStyle::FLockedActorsListStyle，通过统一 getter 提供
- 修改的文件：Plugins/SuperManager/Source/SuperManager/Public/CustomStyle/SuperManagerStyle.h,Plugins/SuperManager/Source/SuperManager/Private/CustomStyle/SuperManagerStyle.cpp,Plugins/SuperManager/Source/SuperManager/Public/SlateWidgets/LockedActorsListWidget.h,Plugins/SuperManager/Source/SuperManager/Private/SlateWidgets/LockedActorsListWidget.cpp,Plugins/SuperManager/Source/SuperManager/Private/SlateWidgets/LockedActorsListRow.cpp

## 实施步骤
1. 在 SuperManagerStyle 中添加 LockedActorsListStyle getter，集中定义颜色/Brush
2. 更新 LockedActorsListWidget/Row 使用新 getter 并清理本地 namespace

## 测试验证
- python3 Scripts/ci/check_agent_guidelines.py
- 测试结果：[AGENTS] 所有检查通过

## 注意事项
- 潜在风险：样式集中管理后若未调用 InitializeIcons 可能访问空指针
- 后续优化：可考虑将颜色暴露到配置以支持主题切换

## 问题提醒
- 以前多个文件重复定义 Brush，任何 UI 调整必须先想起集中入口，避免再次分散

## 下一步建议
- 在 README/样式文档中注明 LockedActorsListStyle 已归属 SuperManagerStyle，新增控件需从此处复用
