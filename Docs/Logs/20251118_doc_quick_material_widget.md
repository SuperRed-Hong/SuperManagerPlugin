# doc quick material widget

## 问题描述
- 具体问题：QuickMaterialCreationWidget 缺少 Doxygen 注释且 RiderLink 三方代码导致脚本误报
- 影响范围：CI Guardrail 脚本持续失败，资产自动化功能无法合规提交

## 解决方案
- 技术方案：为材质创建面板补充注释并让检查脚本忽略 RiderLink 目录
- 修改的文件：Plugins/SuperManager/Source/SuperManager/Public/AssetActions/QuickMaterialCreationWidget.h,Scripts/ci/check_agent_guidelines.py

## 实施步骤
1. 格式化 QuickMaterialCreationWidget.h 行尾与权限
2. 为 enum/class/函数/属性补充 Doxygen 描述
3. 更新检查脚本，跳过 Plugins/Developer/RiderLink

## 测试验证
- python3 Scripts/ci/check_agent_guidelines.py
- 测试结果：脚本返回 [AGENTS] 所有检查通过

## 注意事项
- 潜在风险：脚本当前仅跳过 RiderLink，如后续新增第三方目录需同步配置
- 后续优化：考虑为其他第三方插件建立白名单列表
