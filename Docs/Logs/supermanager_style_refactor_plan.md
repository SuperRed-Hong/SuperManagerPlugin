# SuperManagerStyle 架构重构提案

## 问题描述
- `FSuperManagerStyle` 既持有 `FSlateStyleSet` 生命周期，又暴露大量颜色/Brush 常量，职责混杂，难以判断谁是 Style Set，谁是 Palette 数据。
- 静态名称含糊（如 `IconStyleSetName`、`CreatedSlateStyleSet`），没有描述命名空间和注册范围，易与其他模块样式冲突。
- 初始化流程分散：`InitializeIcons` 仅注册图标，Widget 直接引用静态 `FSlateRoundedBoxBrush`，导致主题切换或统一管理不可行。
- 缺少 Doxygen 注释与生命周期守卫：外部不知道何时调用 `Initialize` / `Shutdown`，也没有复注册保护；模块卸载后访问会触发 `check`。
- 文档缺失，团队成员无法理解扩展 Style 的规范（命名、命令组装、资源路径）。

## 解决方案
- 拆分“数据”与“容器”：新增 `FSuperManagerPalette`（仅存颜色/Brush 数据）与 `FSuperManagerStyleRegistry`（封装 `FSlateStyleSet` 注册/注销）。
- 明确命名：统一使用 `SuperManager.Style` 前缀，常量名改为 `SuperManagerStyleSetName`、`SuperManagerStyleInstance` 等；在头文件添加 Doxygen 注释。
- 初始化时一次性把 Palette 数据注册进 `FSlateStyleSet`，Widget 仅通过 `ISlateStyle::GetBrush()` 访问。
- 增加生命周期守卫（`bIsInitialized`、`ensure`），并在 `StartupModule`/`ShutdownModule` 中严格调用。
- 在 Docs 中固化命名规范、扩展步骤、`FSlateIcon` 使用范例。

## 解决方案
- `FSuperManagerPalette` 维护颜色/Brush 常量，支持未来主题切换。
- `FSuperManagerStyleRegistry`（或 `ExtendEditor::Style::SuperManager` 命名空间）负责 `Initialize`、`Shutdown`、`GetStyleSetName` 与 `Get()`。
- `CreateSlateStyleSet` 注册图标 + Palette Brush，命名采用 `SuperManager.Toolbar.Background` 风格。
- `Initialize` 检查 `bIsInitialized` 并调用 `FSlateStyleRegistry::RegisterSlateStyle`；`Shutdown` 对应 `Unregister`。
- 提供文档与代码注释，确保团队理解使用方式。

## 实施步骤
1. 在 `SuperManagerStyle.h/.cpp` 中新增 `FSuperManagerPalette` 结构，集中声明颜色/Brush。
2. 新建 `FSuperManagerStyleRegistry`（或改造原类）只保留 Style Set 生命周期 API，并增加 Doxygen 注释。
3. 重写 `CreateSlateStyleSet`，把 Palette Brush 也通过 `Set` 注册；Widget 改用 `FSuperManagerStyleRegistry::Get().GetBrush()`。
4. 在 `FSuperManagerModule::StartupModule` 调用新的 `Initialize`，`ShutdownModule` 调用 `Shutdown`。
5. 更新相关 Slate Widget/Outliner Column，移除对静态 Brush 的直接引用。
6. 在 Docs/README 中记录命名规范、扩展流程与测试步骤。

## 测试验证
- 启动 Editor，验证 SuperManager 菜单/Widget 使用到的 Icon、Brush 全部正常。
- 热重载插件或禁用/启用插件，确认 `Initialize`/`Shutdown` 不会重复注册、未触发 `ensure`。
- 运行 `Stat SlateStyle`，确保 Style Set 没有残留或重复。

## 注意事项
- Widget 改为通过 `ISlateStyle` 获取 Brush 后，需要同步更新 `LockedActorsListWidget`、`OutlinerSelectionColumn` 等所有引用点。
- 若未来实现多主题，`FSuperManagerPalette` 应支持从配置加载，当前重构需保留扩展接口。
- Style 注册只允许在 Editor 模块执行，禁止在运行时模块调用 UE Editor API。

## 问题提醒
- “你的提问没有限定项目上下文，容易忽略 ExtendEditor 里‘样式必须集中管理、禁止散落注册’的约束；建议查阅 Docs/UESource 的最新 Slate 头文件确认 API 签名，避免引用旧版路径。”

## 下一步建议
- 由你根据本文方案拆分 `SuperManagerStyle` 并更新受影响 Widget，完成后自测编辑器界面。
- 整理命名规范及扩展指南到 Docs/README，便于其他成员参考。
- 全部修改完成后执行 `git push`。
