# DataLayer 导入功能 - UI 开发文档

> 创建日期: 2025-11-29
> 状态: **方案已确定** - 采用基于 SceneOutliner 框架的自定义浏览器方案
> 最后更新: 2025-11-29
> 预研文档: [DevLog/Phase8_UI_Extension_Research.md](DevLog/Phase8_UI_Extension_Research.md)

---

## 1. 文档目的

本文档整合并更新 DataLayer 导入功能的 UI 设计规格，取代以下过时文档：
- ~~`FeatureSpecs/DataLayerOutlinerExtension_Spec.md`~~ (包含废弃的元数据方案)
- ~~`Specs/PRD.md` 第4节~~ (过于简略)

**重要**: 所有 UI 开发应以本文档为准。

---

## 2. 当前实现状态

### 2.1 已实现功能

| 功能 | 状态 | 文件 |
|------|------|------|
| Stage DataLayer Browser Tab | ✅ 已实现 | `SStageDataLayerBrowser.cpp` |
| 工具栏 - Sync All 按钮 | ✅ 已实现 | 同上 |
| 工具栏 - Import Selected 按钮 | ✅ 已实现 | 同上 |
| 导入预览对话框 | ✅ 已实现 | `SDataLayerImportPreviewDialog.cpp` |
| 事件订阅机制 | ✅ 已实现 | 同上 |

### 2.2 未实现功能（核心问题）

| 功能 | 状态 | 原因 |
|------|------|------|
| **选择同步** | ❌ 无法工作 | 包装器无法获取内嵌 Outliner 的选中项 |
| **Sync Status 列** | ❌ 未实现 | 包装器方案无法添加自定义列 |
| **SUID 列** | ❌ 未实现 | 同上 |
| **Actions 列** | ❌ 未实现 | 同上 |
| **行内按钮** | ❌ 未实现 | 同上 |

### 2.3 当前架构限制

```
当前实现：包装器方案
┌─────────────────────────────────────────────┐
│ SStageDataLayerBrowser                       │
├─────────────────────────────────────────────┤
│ [Sync All] [Import Selected]  命名提示...    │  ← 工具栏（可控）
├─────────────────────────────────────────────┤
│ ┌─────────────────────────────────────────┐ │
│ │         原生 DataLayerBrowser           │ │  ← 黑盒，无法扩展
│ │  (通过 FDataLayerEditorModule 创建)     │ │
│ │                                         │ │
│ │  - 无法添加自定义列                      │ │
│ │  - 无法获取选中项                        │ │
│ │  - 无法 hook 右键菜单                    │ │
│ └─────────────────────────────────────────┘ │
└─────────────────────────────────────────────┘
```

**关键问题**: `FDataLayerMode` 在引擎 Private 目录，插件无法直接使用。

---

## 3. 目标 UI 设计规格

### 3.1 列布局（目标状态）

```
┌─────────────┬──────────────────────┬────────────┬──────────────┐
│ Sync Status │ Item Label (原有)    │ SUID       │ Actions      │
├─────────────┼──────────────────────┼────────────┼──────────────┤
│ ✓ (绿色)    │ DL_Stage_Castle      │ S:1        │ [Sync]       │
│ ⚠ (橙色)    │ DL_Act_Castle_Ext    │ A:1.2      │ [Sync]       │
│ ● (蓝色)    │ DL_Stage_Forest      │            │ [Import]     │
│ (无图标)    │ DL_Other_Random      │            │              │
└─────────────┴──────────────────────┴────────────┴──────────────┘
```

### 3.2 Sync Status 列规格

| 状态 | 图标 | 颜色 | 显示行为 |
|------|------|------|---------|
| **已同步 (Synced)** | ✓ | 绿色 | Hover 时显示，平时隐藏 |
| **不同步 (OutOfSync)** | ⚠ | 橙色 | 始终显示 |
| **未导入 (NotImported)** | ● | 蓝色 | 始终显示（仅符合命名规范的 Stage DataLayer） |
| **非 StageEditor 管理** | (空) | - | 不显示任何图标 |

**Hover 提示内容**（OutOfSync 状态）：

| 变化类型 | 提示文案 |
|----------|---------|
| 新增 Act DataLayer | "检测到新 Act: {名称}，点击同步以导入" |
| 删除 Act DataLayer | "Act '{名称}' 已被删除，点击同步以更新" |
| Actor 成员变化 | "Props 变化：新增 {N} 个，移除 {M} 个" |

### 3.3 SUID 列规格

| 类型 | 格式 | 示例 |
|------|------|------|
| Stage | `S:{StageID}` | `S:1` |
| Act | `A:{StageID}.{ActID}` | `A:1.2` |
| 未导入 | (空) | |

### 3.4 Actions 列规格

| 按钮 | 显示条件 | 功能 |
|------|---------|------|
| **Import** | 符合 `DL_Stage_*` 且未导入 | 打开导入预览对话框 |
| **Sync** | 已导入且 OutOfSync | 同步当前项 |

### 3.5 工具栏规格

| 按钮 | 功能 | 启用条件 |
|------|------|---------|
| **Sync All** | 批量同步所有 OutOfSync 项 | 存在 OutOfSync 的 DataLayer |
| **Import Selected** | 导入选中的 Stage DataLayer | 选中了可导入的 Stage DataLayer |

---

## 4. 实现方案决策

> 详细预研过程见: [DevLog/Phase8_UI_Extension_Research.md](DevLog/Phase8_UI_Extension_Research.md)

### 4.1 备选方案对比

| 方案 | 描述 | 自定义列 | 选中回调 | 稳定性 | 决策 |
|------|------|---------|---------|--------|------|
| A. 运行时 AddColumn | 遍历 Widget 树动态添加列 | ✅ | ❌ | ⚠️ 低 | ❌ 放弃 |
| **B. 基于 SceneOutliner 框架** | 复用框架 + 自定义 Mode/Column | ✅ | ✅ | ✅ 高 | ✅ **采纳** |
| C. Hook InitOptions | 注入 DataLayerBrowser 初始化 | - | - | - | ❌ 不可行 |
| D. 右键菜单扩展 | 通过 GetAllDataLayersMenuExtenders | ❌ | ⚠️ | ✅ 高 | ❌ 放弃 |

> **重要澄清**: 方案 B **不是从零自建 TreeView**，而是复用 UE 的 SceneOutliner 框架。
> 原生 DataLayerBrowser 也是基于同一框架构建的，只是其实现在 Private 目录无法直接使用。
> 我们只需实现业务逻辑层（Mode/Hierarchy/TreeItem/Column），框架提供 TreeView 的所有基础能力。

### 4.2 为什么全局 Column 注册对 DataLayerOutliner 无效？

**关键发现**（源码分析）：

```cpp
// SSceneOutliner.cpp:531-533
if (SharedData->ColumnMap.Num() == 0)
{
    SharedData->UseDefaultColumns();  // 只有 ColumnMap 为空时才加载默认列
}
```

```cpp
// SDataLayerBrowser.cpp:149-165
FSceneOutlinerInitializationOptions InitOptions;
InitOptions.ColumnMap.Add(...);  // DataLayerBrowser 明确添加了自己的列
InitOptions.ColumnMap.Add(...);  // ColumnMap.Num() != 0
```

**结论**: DataLayerBrowser 预设了 `ColumnMap`，不会调用 `UseDefaultColumns()`，全局注册的列不会出现。

### 4.3 最终方案：B - 基于 SceneOutliner 框架构建自定义浏览器

**架构**：
```
┌─────────────────────────────────────────────────────────────┐
│                    SceneOutliner 框架                        │
│  (UE 提供的公开基础设施：TreeView、渲染、选择、拖拽等)         │
└─────────────────────────────────────────────────────────────┘
                              ↑
              ┌───────────────┴───────────────┐
              │                               │
   ┌──────────┴──────────┐         ┌──────────┴──────────┐
   │  原生 DataLayerBrowser │         │  我们的 StageDataLayer │
   │  (引擎 Private 实现)   │         │  Browser (自建)        │
   ├─────────────────────┤         ├─────────────────────┤
   │ FDataLayerMode      │         │ FStageDataLayerMode │
   │ DataLayerHierarchy  │         │ StageDataLayerHierarchy │
   │ DataLayerTreeItem   │         │ StageDataLayerTreeItem │
   │ 原生 Columns        │         │ 自定义 Columns       │
   └─────────────────────┘         └─────────────────────┘
```

**详细架构**：
```
SStageDataLayerBrowser (重构)
├── 工具栏 [Sync All] [Import Selected]
└── SSceneOutliner (UE 公开框架，提供 TreeView 基础能力)
    ├── FStageDataLayerMode : ISceneOutlinerMode
    │   ├── 数据源：UDataLayerManager
    │   ├── 过滤：只显示 DataLayer
    │   └── 行为：选择、展开、右键菜单
    │
    ├── FStageDataLayerHierarchy : ISceneOutlinerHierarchy
    │   └── 构建 DataLayer 树结构
    │
    ├── FStageDataLayerTreeItem : ISceneOutlinerTreeItem
    │   └── 表示单个 DataLayer 节点
    │
    └── 自定义列
        ├── FStageDataLayerSyncStatusColumn
        ├── FStageDataLayerSUIDColumn
        └── FStageDataLayerActionsColumn
```

**SceneOutliner 框架提供的能力**（我们不需要实现）：
| 能力 | 说明 |
|------|------|
| TreeView 渲染 | 行绘制、虚拟滚动、缓存 |
| 选择系统 | 单选/多选、Shift/Ctrl 修饰键 |
| 展开/折叠 | 树节点状态管理 |
| 拖拽基础 | 拖拽检测、视觉反馈 |
| 列系统 | HeaderRow、列布局、列排序 |

**我们需要实现的**（业务逻辑层）：
| 类 | 预计代码量 | 职责 |
|----|-----------|------|
| FStageDataLayerMode | ~200-300 行 | 定义数据源和行为 |
| FStageDataLayerHierarchy | ~100-200 行 | 定义树的层级结构 |
| FStageDataLayerTreeItem | ~150-250 行 | 定义节点类型 |
| 自定义 Columns (4个) | ~280-450 行 | 自定义列渲染 |
| **总计** | **~700-1200 行** | |

**优点**：
- 完全可控，可实现所有目标 UI 功能
- 使用公开 API，不依赖 Private 实现
- 升级安全，维护简单
- **工作量可控**：复用框架能力，非从零实现

**接受的代价**：
- 需要理解 SceneOutliner 框架的接口设计
- 需要与原生 DataLayer 系统保持数据同步

---

## 5. 实施路线图

### Phase 8.1: 基础架构
- [ ] 创建 `FStageDataLayerHierarchy` - 数据层级结构
- [ ] 创建 `FStageDataLayerTreeItem` - 树节点类型
- [ ] 创建 `FStageDataLayerMode` - Outliner 模式

### Phase 8.2: 自定义列
- [ ] 创建 `FStageDataLayerLabelColumn` - 名称列
- [ ] 创建 `FStageDataLayerSyncStatusColumn` - 同步状态列
- [ ] 创建 `FStageDataLayerSUIDColumn` - SUID 列
- [ ] 创建 `FStageDataLayerActionsColumn` - 操作按钮列

### Phase 8.3: 集成
- [ ] 重构 `SStageDataLayerBrowser` 使用自建 Outliner
- [ ] 实现选中回调
- [ ] 实现右键菜单
- [ ] 事件同步（DataLayer 变化时刷新）

### Phase 8.4: 功能完善
- [ ] 拖拽支持（可选）
- [ ] 搜索过滤
- [ ] 列可见性持久化

---

## 6. 设计决策

> 这些决策基于核心目标：**用 StageEditor 接管所有 UE DataLayer 的管理**

### Q1: 与原生 DataLayerBrowser 的关系
**决策**: 使用上替代，数据上同步

- 对 StageEditor 用户来说，Stage DataLayer Browser 是主要入口
- 但无法阻止用户使用原生 DataLayerBrowser，所以数据必须与原生系统保持一致
- 我们的浏览器显示的是 UDataLayerManager 的真实数据

### Q2: 显示哪些 DataLayer？
**决策**: 显示全部 DataLayer

- 核心功能就是把**未被 StageEditor 管理的 DataLayer** 导入进来
- 所有扩展功能（Sync Status、Import、Rename）都是为未管理的 DataLayer 设计的
- 已管理的显示同步状态，未管理的显示导入按钮

### Q3: Rename 功能
**决策**: 保留，实现快速重命名

- 右键菜单提供 "Rename to Stage Format" / "Rename to Act Format"
- 帮用户快速将任意 DataLayer 重命名为 `DL_Stage_*` / `DL_Act_*` 格式
- 这是用户体验的关键功能

### Q4: 原生编辑操作（创建/删除/重命名）
**决策**: 不支持

- 创建/删除操作由 StageEditorPanel 接管（通过 Stage-Act 管理）
- 重命名有专门的快捷按钮
- 保持浏览器职责单一：查看状态 + 导入/同步

---

## 7. 相关文件索引

### 7.1 现有实现
```
Plugins/StageEditor/Source/StageEditor/
├── Public/DataLayerSync/
│   ├── SStageDataLayerBrowser.h           # 当前包装器实现
│   ├── SDataLayerImportPreviewDialog.h    # 导入预览对话框
│   ├── DataLayerSyncStatus.h              # 状态枚举和检测器
│   └── DataLayerSyncStatusCache.h         # 状态缓存
│
├── Private/DataLayerSync/
│   ├── SStageDataLayerBrowser.cpp
│   └── SDataLayerImportPreviewDialog.cpp
```

### 7.2 参考实现（SuperManager）
```
Plugins/SuperManager/Source/SuperManager/
├── Public/
│   └── SuperManager.h                     # SceneOutliner Column 注册
│
├── Private/
│   └── ... (LockedActors Column 实现)
```

### 7.3 引擎源码参考（只读）
```
Engine/Source/Editor/DataLayerEditor/       # 引擎 DataLayer 编辑器
├── Public/
│   ├── DataLayerEditorModule.h
│   └── DataLayerEditorSubsystem.h
│
├── Private/
│   ├── SDataLayerBrowser.cpp              # DataLayer 浏览器实现
│   ├── SDataLayerOutliner.cpp             # Outliner 实现
│   ├── DataLayerMode.h/cpp                # Mode 实现（Private）
│   └── DataLayerOutlinerColumns.cpp       # 列实现（Private）
```

---

## 8. 废弃内容说明

以下内容已废弃，**不要参考**：

| 废弃内容 | 原位置 | 原因 |
|----------|--------|------|
| UAssetUserData 元数据方案 | `FeatureSpecs/DataLayerOutlinerExtension_Spec.md` 第5节 | UDataLayerAsset 不支持 AssetUserData |
| GetSyncStatusInfo 基于元数据的实现 | 同上 第5.3节 | 已改用反向查找方案 |
| 实施计划 Phase 1-6 | 同上 第9节 | 与实际开发路径不符 |

**正确的技术方案**请参考：
- `DevLog/Architecture_ReverseLookup.md` - 反向查找架构
- `Specs/TechSpec.md` 第2-4节 - 反向查找 API

---

## 9. 下一步行动

1. ~~**讨论确认**~~：✅ 已确认采用方案 B（基于 SceneOutliner 框架）
2. **技术预研**：研究 SceneOutliner Mode/Hierarchy/TreeItem API
3. **原型开发**：实现最小可行版本（基础 TreeView + Sync Status 列）
4. **迭代完善**：添加 SUID 列、Actions 列

---

## 10. 未来待评估

### TODO: StageEditorPanel 重构评估

> **触发条件**: Phase 8 完成后评估

完成 DataLayerBrowser 基于 SceneOutliner 框架的重构后，评估是否值得将 StageEditorPanel 也迁移到同一框架。

**潜在收益**：
- 代码统一性：两个 Panel 使用相同框架
- 维护性：统一模式，降低学习成本
- 扩展性：未来添加列更方便

**需要解决的问题**：
- FAct 是 struct 而非 UObject，需要适配
- 文件夹节点是纯 UI 概念，需要创建虚拟 TreeItem
- Hierarchy 需要从 AStage 构建而非从 World 获取

**评估时机**: Phase 8 完成后，基于实际开发经验决策

---

*文档版本: v1.2*
*创建日期: 2025-11-29*
*更新记录:*
- *v1.1: 确定采用方案 B，更新实施路线图*
- *v1.2: 澄清方案 B 是"复用 SceneOutliner 框架"而非"从零自建 TreeView"，更新架构图和工作量估算*
