# SuperManager Window 菜单重复入口修复

## 问题描述
- Window 菜单下 “Locked Actors List / Todo List” 各出现两条入口。
- 影响范围：所有启用 SuperManager 插件的编辑器实例，用户每次启动都会看到重复菜单，易造成误操作。

## 解决方案
- 复查 `FSuperManagerModule::InitLevelEditorMenuExtension`，确认重复入口来自手动添加 `Section.AddMenuEntry`。
- 保留 Nomad Tab 的默认注册，移除多余的 Window 菜单入口；右键 Actor 菜单及命令映射不受影响。
- 修改文件：`Plugins/SuperManager/Source/SuperManager/Private/SuperManager.cpp`。

## 实施步骤
1. 打开 `SuperManager.cpp`，定位 `InitLevelEditorMenuExtension`。
2. 删除 `Section.AddMenuEntry(FSuperManagerUICommands::Get().OpenLockedActorsListCommand);` 与 `OpenTodoListCommand` 这两行。
3. 保存文件，准备编译并验证（待用户执行）。

## 测试验证
- 重启 Unreal Editor，展开 Window 菜单：每个 SuperManager 工具仅保留一条入口。
- 右键选中 Actor，确认 “Lock Actor Selection / Unlock All Actors Selection” 仍在 Actor Context Menu 中。

## 注意事项
- 若后续需要完全自定义 Window 菜单，请记得在注册 Nomad Tab 时调用 `.SetMenuType(ETabSpawnerMenuType::Hidden)`，避免再次重复。
- 热重载时仍需在 `ShutdownModule` 里对 Tab Spawner 做成对 Unregister，以免残留。

## 问题提醒
- “你的提问没有限定项目上下文，容易忽略 ExtendEditor 里‘样式必须集中管理、禁止散落注册’的约束；建议查阅 Docs/UESource 的最新 Slate 头文件确认 API 签名，避免引用旧版路径。”

## 下一步建议
- 1. 由你在本地重新编译并运行 Editor，确认 Window 菜单没有重复入口。
- 2. 自查其他模块是否存在重复注册行为（例如自定义命令绑到多个菜单）。
- 3. 所有更改完成后执行 `git push`。
