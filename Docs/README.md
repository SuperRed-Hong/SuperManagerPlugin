# SuperManager Style 扩展指南

## 命名规范
- Style Set 统一使用 `SuperManager.Style` 命名空间，Brush Key 采用 `SuperManager.Style.[区域].[用途]` 模式，例如 `SuperManager.Style.Card.Primary`。
- 场景编辑器图标保持原有键名（如 `ContentBrowser.DeleteUnusedAssets`），仅依赖新的 Style Set 名称。
- 所有新增颜色或 Brush 必须在 `FSuperManagerPalette::CreateDefault` 中声明，同时在 `RegisterPaletteBrushes` 中注册，以便生命周期由注册表控制。

## 扩展流程
1. **添加 Palette 数据**：在 `FSuperManagerPalette::CreateDefault` 中定义颜色/Brush，确保复用现有常量避免 Magic Number。
2. **注册 Style 键**：在 `FSuperManagerStyleNames` 中新增 `FName`，并在 `RegisterPaletteBrushes` 中调用 `StyleSet.Set(Name, Brush)`。
3. **初始化**：无需手动注册，`FSuperManagerStyleRegistry::Initialize()` 会在模块启动时统一执行。
4. **控件使用**：Widget 通过 `FSuperManagerStyleRegistry::GetBrush(Key)` 获取 Brush，通过 `GetPalette()` 获取颜色；禁止在 Widget 中声明新的静态 Brush。
5. **命令/图标**：注册 `FSlateIcon` 时始终调用 `FSuperManagerStyleRegistry::GetStyleSetName()` 作为命名空间，保证菜单与工具栏在主题切换时自动刷新。

## 测试步骤
- 启动 Editor 并打开 SuperManager 相关 Tab，确认新旧 Brush 全部生效。
- 切换/禁用插件或执行模块热重载，验证 `Initialize`/`Shutdown` 未触发 `ensure`，`Stat SlateStyle` 中仅存在单个 `SuperManager.Style` 条目。
- 如新增颜色，检查按钮/文本在 Hover/Pressed 状态下的可读性，必要时调整 Alpha。

## 注意事项
- Style 注册只能在 Editor 模块内执行；Runtime 模块禁止直接访问 `FSuperManagerStyleRegistry`。
- 若需支持主题切换，可扩展 `FSuperManagerPalette` 让颜色从配置加载，但依然通过注册表统一刷新。
- 每次在回复中新增“下一步建议”时同步更新 `Docs/Logs` 记录，确保沟通一致。
