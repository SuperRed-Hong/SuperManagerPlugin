# refine locked actors style access

## 问题描述
- 具体问题：需要用统一 SuperManagerStyle 静态成员管理 LockedActors UI 颜色/Brush
- 影响范围：SuperManager 锁定面板所有 Slate

## 解决方案
- 技术方案：将 LockedActorsListStyle 改为 FSuperManagerStyle 的 static 成员，并更新 Widget/Row 引用
- 修改的文件：Plugins/SuperManager/Source/SuperManager/Public/CustomStyle/SuperManagerStyle.h,Plugins/SuperManager/Source/SuperManager/Private/CustomStyle/SuperManagerStyle.cpp,Plugins/SuperManager/Source/SuperManager/Private/SlateWidgets/LockedActorsListWidget.cpp,Plugins/SuperManager/Source/SuperManager/Private/SlateWidgets/LockedActorsListRow.cpp

## 实施步骤
1. 在 SuperManagerStyle 声明 static 颜色与 RoundedBoxBrush
2. 移除旧 getter/namespace，实现静态成员
3. 更新 LockedActorsListWidget/Row 使用静态成员+按钮字体改为白色

## 测试验证
- python3 Scripts/ci/check_agent_guidelines.py
- 测试结果：[AGENTS] 所有检查通过

## 注意事项
- 潜在风险：样式集中于静态成员，若忘记初始化 StyleSet 可能访问空 brush
- 后续优化：未来若想做主题化，可把 static 改回 getter 或从配置加载

## 问题提醒
- 之前 getter 过多导致使用繁琐，本次改回静态成员但仍需集中管理，别再在外部写 literal

## 下一步建议
- 在 README/样式章节上提及 FSuperManagerStyle::LockedActors* 常量，提醒团队复用
