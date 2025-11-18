# fix locked actors widget compile

## 问题描述
- 具体问题：LockedActorsListWidget 常量与函数重复定义导致 UE5.6 编译器报错
- 影响范围：SuperManager 插件 LockedActors 列表功能编译失败，编辑器面板不可用

## 解决方案
- 技术方案：合并 Brush/颜色常量定义、修复 Doxygen 注释，调整行控件调用入口并删除重复声明
- 修改的文件：Plugins/SuperManager/Source/SuperManager/Public/SlateWidgets/LockedActorsListWidget.h,Plugins/SuperManager/Source/SuperManager/Private/SlateWidgets/LockedActorsListWidget.cpp,Plugins/SuperManager/Source/SuperManager/Private/SlateWidgets/LockedActorsListRow.cpp

## 实施步骤
1. 移除 .cpp 内静态常量，统一在头文件 inline 定义
2. 修复 Brush 初始化语法与参数，补充 Doxygen
3. 更新 Row 控件调用 RequestLockStateChange，删除重复声明

## 测试验证
- python3 Scripts/ci/check_agent_guidelines.py (因历史文件缺注释失败)
- 人工检查 LockedActorsListWidget.h/Row.cpp 语法
- 测试结果：脚本检测触发旧文件告警，自身修改区域无新错误

## 注意事项
- 潜在风险：CI 检查脚本对旧文件仍有大量告警，需要后续专门清理
- 后续优化：建议下一步逐步为 RiderLink/SuperManager 旧头文件补注释
