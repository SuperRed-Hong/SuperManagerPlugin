# Phase 10: Import 功能重设计

> 日期: 2025-11-30
> 状态: ✅ 全部完成 (Phase 10.1-10.5)
> 优先级: HIGH
> 讨论确认: 2025-11-30

---

## 1. 需求背景

### 1.1 用户反馈的问题

1. **行内 Import 与 ToolBar ImportSelected 功能重复**
   - 两个按钮触发完全相同的流程，用户体验冗余

2. **Preview 对话框 SUID 列显示 "?"**
   - Import 前 Stage 未创建，无法获取实际 StageID

3. **Import Stage 不会自动导入子 Act DataLayers**
   - 需要导入 Stage 时固定把旗下所有 ActDataLayer 跟随导入
   - 单独导入 ActDataLayer 无意义

4. **DefaultAct 未适配**
   - StageEditorPanel 创建的 Stage 有 DefaultAct
   - Import 创建的 Stage 缺少 DefaultAct，数据结构不一致

---

## 2. 设计方案（已确认）

### 2.1 Import 按钮体系重定义

| 按钮 | 位置 | 目标 | 行为 |
|------|------|------|------|
| **Import** | 行内 Actions 列 | 单个 Stage | Preview 对话框 → 确认 → 导入 |
| **Import Selected** | ToolBar | 批量 Stages | 批量 Preview 对话框 → 确认 → 导入 |
| ~~Import~~ | Act 行 Actions 列 | - | **移除**（单独导入 Act 无意义） |

### 2.2 子 Act 跟随导入

#### 判断规则
1. 按 DataLayer **层级关系**（Parent-Child）判断子层
2. 检查命名规范（DL_Act_*）
3. **不符合规范时弹警告**，用户可选择：
   - 继续导入
   - 取消整个导入操作

#### 处理流程
```
导入 Stage DataLayer 时:
1. 获取所有 Parent 为该 Stage 的子 DataLayers
2. 遍历每个子层:
   - 检查命名规范 (DL_Act_<StageName>_<ActName>)
   - 符合 → 加入导入列表
   - 不符合 → 记录警告
3. 如有不符合规范的子层:
   - 弹出警告对话框
   - 用户选择 [继续导入] / [取消]
4. 全部子 Acts 跟随 Stage 一起导入
```

### 2.3 DefaultAct 适配

#### 设计原则
- 每个 Stage 必须有 DefaultAct (ID=1)
- 保持与 StageEditorPanel 创建的 Stage 结构一致

#### 两种情况处理

| 情况 | 处理方式 |
|------|----------|
| **有子 ActDataLayer** | Preview 中显示下拉选择框，用户选择一个作为 DefaultAct |
| **无子 ActDataLayer** | 自动创建空的 DefaultAct (ID=1, 无关联 DataLayer) |

#### DefaultAct 选择行为
- 被选中的 ActDataLayer **关联到 DefaultAct (ID=1)**
- **保留原 DisplayName**（不改名为 "Default Act"）
- 其他 Acts 从 ID=2 开始

```
示例：用户选择 "Combat" 作为 DefaultAct

导入前的 DataLayer 结构:
├── DL_Stage_MainStage
│   ├── DL_Act_MainStage_Combat
│   └── DL_Act_MainStage_Peaceful

导入后的 Stage.Acts:
┌───────┬───────┬─────────────┬─────────────────────────────┐
│ Index │ ActID │ DisplayName │ AssociatedDataLayer         │
├───────┼───────┼─────────────┼─────────────────────────────┤
│   0   │   1   │ "Combat"    │ DL_Act_MainStage_Combat     │  ← DefaultAct
│   1   │   2   │ "Peaceful"  │ DL_Act_MainStage_Peaceful   │
└───────┴───────┴─────────────┴─────────────────────────────┘
```

### 2.4 ToolBar 批量 Import Preview 设计

#### 混合选中处理（智能合并）
- Stage DataLayers → 正常导入
- 单独选中的 Act DataLayers：
  - 如果其 Parent Stage **已选中** → 忽略（随 Stage 导入）
  - 如果其 Parent Stage **未选中** → 警告跳过

#### UI 布局：树状 + 内联下拉

```
┌─────────────────────────────────────────────────────────┐
│  批量导入预览                                            │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ├── Stage: MainStage                                   │
│  │   ├── DefaultAct: [Combat        ▼]   ← 下拉选择     │
│  │   ├── Act: Combat (DL_Act_MainStage_Combat)          │
│  │   └── Act: Peaceful (DL_Act_MainStage_Peaceful)      │
│  │                                                      │
│  ├── Stage: BossRoom                                    │
│  │   ├── DefaultAct: [Phase1        ▼]   ← 下拉选择     │
│  │   ├── Act: Phase1 (DL_Act_BossRoom_Phase1)           │
│  │   └── Act: Phase2 (DL_Act_BossRoom_Phase2)           │
│  │                                                      │
│  ⚠️ 跳过: DL_Act_Orphan (Parent Stage 未选中)            │
│                                                         │
│  ─────────────────────────────────────────────────────  │
│  统计: 2 Stages, 4 Acts                                 │
│                                                         │
│                         [取消]  [导入]                   │
└─────────────────────────────────────────────────────────┘
```

#### 单个 Stage Import Preview（行内按钮）

```
┌─────────────────────────────────────────────────────────┐
│  导入预览: MainStage                                     │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  Stage: MainStage (来自 DL_Stage_MainStage)             │
│                                                         │
│  DefaultAct: [Combat        ▼]   ← 下拉选择             │
│                                                         │
│  将导入的 Acts:                                          │
│  ├── Combat (DL_Act_MainStage_Combat)                   │
│  └── Peaceful (DL_Act_MainStage_Peaceful)               │
│                                                         │
│  ─────────────────────────────────────────────────────  │
│  统计: 1 Stage, 2 Acts                                  │
│                                                         │
│                         [取消]  [导入]                   │
└─────────────────────────────────────────────────────────┘
```

---

## 3. 实施计划

### Phase 10.1: 移除 Act 行 Import 按钮 ✅
**优先级**: HIGH
**文件**: `StageDataLayerColumns.cpp`
**状态**: 已完成 (2025-11-30)

**实现说明**:
- 在 `FStageDataLayerActionsColumn::ConstructRowWidget()` 的 `NotImported` case 中添加命名解析判断
- 只有 Stage 级别 DataLayer (`ParseResult.bIsStageLayer`) 或非标准命名 (`!ParseResult.bIsValid`) 显示 Import 按钮
- Act 级别 DataLayer 不显示 Import 按钮（将随 Parent Stage 一起导入）

```cpp
case EDataLayerSyncStatus::NotImported:
{
    // Only show Import button for Stage-level DataLayers (not Act-level)
    FDataLayerNameParseResult ParseResult = FStageDataLayerNameParser::Parse(DataLayerAsset->GetName());
    if (!ParseResult.bIsValid || ParseResult.bIsStageLayer)
    {
        // Show Import for: Stage DataLayers OR non-standard naming (user decides)
        // ... Import button ...
    }
    // Act-level DataLayers: no Import button (will be imported with parent Stage)
    break;
}
```

### Phase 10.2: 子 Act 跟随导入 + 命名规范警告 ✅
**优先级**: HIGH
**文件**: `DataLayerImporter.h/cpp`, `StageEditorController.cpp`, `SDataLayerImportPreviewDialog.h/cpp`
**状态**: 已完成 (2025-11-30)

**实现说明**:

1. **新增 `FDataLayerNamingWarning` 结构体** (`DataLayerImporter.h`)
   - 记录不规范命名的 DataLayer 信息
   - 包含 `DataLayerName`, `DataLayerAsset`, `WarningReason`

2. **修改 `FDataLayerImportPreview`** (`DataLayerImporter.h`)
   - 添加 `NamingWarnings` 数组
   - 添加 `HasNamingWarnings()` 辅助方法

3. **修改 `GeneratePreview()`** (`DataLayerImporter.cpp`)
   - 收集所有子层（不限命名规范）
   - 检查命名规范，记录警告原因：
     - 不符合命名规范 (expected: DL_Act_<StageName>_<ActName>)
     - 命名为 Stage 但作为子层 (treating as Act)
     - Act 命名引用不同 Stage

4. **修改 `ImportStageFromDataLayer()`** (`StageEditorController.cpp`)
   - 导入所有子层（不限命名规范）
   - 根据命名解析结果确定 DisplayName

5. **修改 Preview 对话框** (`SDataLayerImportPreviewDialog.h/cpp`)
   - 添加 `BuildWarningContent()` 方法
   - 在 UI 中显示黄色警告框，列出不规范命名的 DataLayers
   - 显示提示：这些 DataLayers 仍会被导入，建议后续重命名

### Phase 10.3: DefaultAct 适配 ✅
**优先级**: HIGH
**文件**: `DataLayerImporter.h/cpp`, `StageEditorController.h/cpp`
**状态**: 已完成 (2025-11-30)

**实现说明**:

1. **新增 `FDataLayerImportParams` 结构体** (`DataLayerImporter.h`)
   - 包含 `StageDataLayerAsset` 和 `SelectedDefaultActIndex`
   - `SelectedDefaultActIndex`: -1 表示空 DefaultAct，0+ 表示选择的子 DataLayer 索引

2. **新增 `ExecuteImport(FDataLayerImportParams)` 重载** (`DataLayerImporter.cpp`)
   - 传递 DefaultAct 选择参数到 Controller

3. **新增 `ImportStageFromDataLayerWithDefaultAct()` 方法** (`StageEditorController.h/cpp`)
   - 接受 `SelectedDefaultActIndex` 参数
   - 先收集所有子 DataLayers 到数组
   - 根据选择更新或保留 DefaultAct (Acts[0])

4. **DefaultAct 处理逻辑**:
   - **有子 Acts 且 Index >= 0**: 选中的子 DataLayer 更新到 Acts[0]，其他从 ActID=2 开始
   - **有子 Acts 且 Index == -1**: 保留空 DefaultAct，子 DataLayers 从 ActID=2 开始
   - **无子 Acts**: 保留构造函数创建的空 DefaultAct

5. **原方法兼容**: `ImportStageFromDataLayer()` 调用新方法，默认选择第一个子 DataLayer (Index=0)

### Phase 10.4: Preview 对话框重构 ✅
**优先级**: HIGH
**文件**: `SDataLayerImportPreviewDialog.h/cpp`
**状态**: 已完成 (2025-11-30)

**实现说明**:

1. **新增 `FDefaultActOption` 结构体** (`SDataLayerImportPreviewDialog.h`)
   - 用于 DefaultAct 下拉选择框的选项
   - 包含 `DisplayName`, `ChildDataLayerIndex`, `DataLayerAsset`

2. **添加 DefaultAct 下拉选择功能**:
   - `InitializeDefaultActOptions()`: 从 Preview 收集 Act 项构建选项列表
   - `BuildDefaultActSelector()`: 构建 SComboBox UI
   - `GenerateDefaultActOptionWidget()`: ComboBox 行生成
   - `OnDefaultActSelectionChanged()`: 选择变更处理
   - `GetSelectedDefaultActText()`: 显示当前选中项

3. **移除无意义的 SUID 列**:
   - 旧实现使用 SListView + SMultiColumnTableRow 显示 "S:?" 等无意义 SUID
   - 新实现直接使用 SScrollBox + STextBlock 展示 Acts 列表

4. **优化 UI 布局**:
   - 标题显示 Stage 名称
   - Stage 信息区（名称 + 来源 DataLayer）
   - DefaultAct 选择区（SComboBox + 提示文本）
   - Acts 列表区（树状缩进 ├── └──）
   - 无子 Acts 时显示提示信息
   - 命名警告区（保留原有黄色警告框）
   - 统计汇总 + 按钮

5. **修改 `OnImportClicked()`**:
   - 使用 `FDataLayerImportParams` 传递 `SelectedDefaultActIndex`
   - 调用 `FDataLayerImporter::ExecuteImport(Params, World)`

### Phase 10.5: 批量 Import Preview 新建 ✅
**优先级**: MEDIUM
**新文件**: `SBatchImportPreviewDialog.h/cpp`
**状态**: 已完成 (2025-11-30)

**实现说明**:

1. **新增 `FBatchImportStageEntry` 结构体**
   - 包含 Stage DataLayer 信息、子 Acts 列表、DefaultAct 选项、命名警告
   - 每个 Stage 独立管理 `SelectedDefaultActIndex`

2. **新增 `FSkippedDataLayerInfo` 结构体**
   - 记录被跳过的 DataLayer 名称和原因

3. **智能合并逻辑 (`ProcessInputDataLayers`)**:
   - 第一遍: 收集所有可导入的 Stage DataLayers
   - 第二遍: 检查 Act DataLayers:
     - Parent Stage 已选中 → 跳过（随 Stage 导入）
     - Parent Stage 未选中 → 记录跳过原因
     - 非标准命名 → 记录跳过原因

4. **UI 布局**:
   - 树状 Stage 列表（├── └── 缩进）
   - 每个 Stage 独立的 DefaultAct 下拉选择器
   - Acts 子列表显示
   - 命名警告指示器
   - 跳过项警告区（黄色背景）
   - 统计汇总 + Import All 按钮

5. **修改 `SStageDataLayerOutliner`**:
   - 新增 `GetSelectedImportableDataLayerAssets()` 返回所有可导入的 Assets
   - `OnImportSelectedClicked()` 根据选中数量调用对应对话框:
     - 1 个 → `SDataLayerImportPreviewDialog`
     - 多个 → `SBatchImportPreviewDialog`

---

## 4. 代码变更清单

| 文件 | 变更类型 | 说明 |
|------|----------|------|
| `StageDataLayerColumns.cpp` | 修改 | Act 行移除 Import 按钮 |
| `StageEditorController.cpp` | 修改 | DefaultAct 适配，子 Act 跟随导入 |
| `DataLayerImporter.cpp` | 修改 | 命名规范警告，Preview 数据生成 |
| `SDataLayerImportPreviewDialog.cpp` | 修改 | 添加 DefaultAct 下拉选择 |
| `SBatchImportPreviewDialog.h/cpp` | **新建** | 批量导入预览对话框 |
| `SStageDataLayerOutliner.cpp` | 修改 | ToolBar ImportSelected 调用批量 Preview |

---

## 5. 验收标准

### 功能验收

- [x] Stage DataLayer 行显示 Import 按钮 (10.1 ✅)
- [x] Act DataLayer 行**不显示** Import 按钮 (10.1 ✅)
- [x] 行内 Import 弹出单 Stage Preview（含 DefaultAct 下拉）(10.4 ✅)
- [x] ToolBar Import Selected 弹出批量 Preview（含多个 DefaultAct 下拉）(10.5 ✅)
- [x] 子 Act 命名不规范时弹出警告 (10.2 ✅)
- [x] 导入后 Stage.Acts[0] 为用户选择的 DefaultAct (ID=1) (10.3 ✅)
- [x] 无子 Act 时自动创建空 DefaultAct (10.3 ✅)

### 回归测试

- [ ] StageEditorPanel 创建 Stage 功能正常
- [ ] Sync 按钮功能正常
- [ ] Std Rename 按钮功能正常

---

## 6. 附录

### 当前代码路径

**行内 Import**:
```
StageDataLayerColumns.cpp
└── FStageDataLayerActionsColumn::OnImportClicked()
    └── SDataLayerImportPreviewDialog::ShowDialog()
        └── FDataLayerImporter::ExecuteImport()
            └── FStageEditorController::ImportStageFromDataLayer()
```

**ToolBar ImportSelected**:
```
SStageDataLayerOutliner.cpp
└── OnImportSelectedClicked()
    └── SDataLayerImportPreviewDialog::ShowDialog()  ← 需改为批量 Preview
        └── ...
```

---

*文档创建: 2025-11-30 14:45*
*讨论确认: 2025-11-30 15:00*
