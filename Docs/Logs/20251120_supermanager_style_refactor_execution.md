# SuperManager Style 解耦与生命周期治理

## 问题描述
- 现有 `FSuperManagerStyle` 同时维护 `FSlateStyleSet` 生命周期与所有颜色/Brush 常量，Widget 直接访问静态 Brush，导致主题切换和统一资源命名不可行。
- 初始化函数仅注册图标，模块关闭后缺少 `ensure` 守卫，重复注册或野指针访问容易触发崩溃；外部也无法了解命名规范。

## 解决方案
- **技术方案文档**：引入 `FSuperManagerPalette`（仅存颜色与 `FSlateBrush`）和 `FSuperManagerStyleRegistry`（封装 Style Set 注册/注销/查询），生命周期由注册表集中管理；Widget 通过 `ISlateStyle::GetBrush` 访问。
- **数据模型定义**：Palette 由 `struct FSuperManagerPalette` 组成，公开颜色和 Brush 字段，未来可从配置热加载；`FSuperManagerStyleRegistry` 持有 `TSharedPtr<FSlateStyleSet>` 与 `bool bIsInitialized`。
- **接口契约**：`FSuperManagerStyleRegistry::Initialize()`、`Shutdown()`、`IsInitialized()`、`Get()`、`GetStyleSetName()`、`RegisterPalette(const FSuperManagerPalette&)` 构成 Editor 模块间的 API。所有命名使用 `SuperManager.Style.*` 命名空间避免冲突。
- **规范遵循检查表**：遵守根 `AGENTS.md` 的命名/注释要求；禁止在非 Editor 模块注册 Style；所有公有接口带 Doxygen 注释；日志记录使用 UE 的宏；禁止直接引用 `Docs/UESource` 内容。
- **修改的文件**：`Plugins/SuperManager/Public/CustomStyle/SuperManagerStyle.h`、`Plugins/SuperManager/Private/CustomStyle/SuperManagerStyle.cpp`、`Plugins/SuperManager/Private/SuperManager.cpp`、`Plugins/SuperManager/Public/SlateWidgets/LockedActorsListWidget.h`、`Plugins/SuperManager/Private/SlateWidgets/LockedActorsListWidget.cpp`、`Plugins/SuperManager/Private/SlateWidgets/LockedActorsListRow.cpp`、`Plugins/SuperManager/Private/CustomOutlinerColumn/OutlinerSelectionColumn.cpp`、`Docs/README.md`。

## 实施步骤
1. 在 `SuperManagerStyle.h/.cpp` 中新增 `FSuperManagerPalette`，集中声明颜色和 Brush，并提供 `BuildPalette()` 工厂。
2. 将原 `FSuperManagerStyle` 拆分为 `FSuperManagerStyleRegistry`，持有 `TSharedPtr<FSlateStyleSet>`、`bIsInitialized` 以及新的 `CreateSlateStyleSet(const FSuperManagerPalette&)`。
3. 所有 Widget、Outliner 列、命令注册均改为 `FSuperManagerStyleRegistry::Get()`/`GetBrush()` 访问，而不是引用静态 Brush 变量。
4. `FSuperManagerModule::StartupModule/ShutdownModule` 调整为调用新的 `Initialize`/`Shutdown`，并确保顺序早于 UI 命令注册。
5. 更新 `Docs/README`（若缺失则新增条目）阐述命名规范、资源路径、扩展流程及测试步骤。
6. 按模板记录 `问题提醒` 与 `下一步建议`，确保团队同步信息。

## 测试验证
- 启动 Editor，检查 SuperManager 菜单/Widget 的 Icon 与 Brush 是否仍然渲染；执行热重载以确认注册表守卫阻止重复注册。
- 使用 `Stat SlateStyle` 或 `SlateStyleRegistry::FindSlateStyle` 检查是否存在残留 Style Set，并验证插件禁用/启用后没有 `ensure`。

## 注意事项
- `FSuperManagerPalette` 需保持可扩展接口，后续主题切换可在此注入配置；`FSuperManagerStyleRegistry` 严禁在非 Editor 线程调用。
- 由于 Widget 依赖新的 Style 访问方式，必须更新所有 `FSuperManagerStyle::Primary*` 用法以免编译错误。

## 问题提醒
- 原文抄录：`你的提问没有限定项目上下文，容易忽略 ExtendEditor 里‘样式必须集中管理、禁止散落注册’的约束；建议查阅 Docs/UESource 的最新 Slate 头文件确认 API 签名，避免引用旧版路径。`

## 下一步建议
- 完成实现后运行 Editor 做视觉回归；整理 Style 扩展指南到 `Docs/README` 并在 PR 中提醒评审关注命名规范。
