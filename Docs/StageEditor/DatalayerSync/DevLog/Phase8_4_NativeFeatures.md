# Phase 8.4: 原生 DataLayer Outliner 功能迁移

> 日期: 2025-11-29
> 状态: ✅ 核心功能完成
> 任务: 将原生 DataLayer Outliner 的核心功能迁移到 Stage DataLayer Outliner
> 最后更新: 2025-11-30 01:05

---

## 背景

Phase 8.1-8.3 完成了基于 SceneOutliner 框架的自定义 DataLayer 浏览器基础架构。用户测试后提出需要部分原生 DataLayer Outliner 功能，本 Phase 将按优先级迁移这些功能。

---

## 功能需求清单

### 高优先级（必须实现）

#### 1. IsVisible 列（眼睛图标）
**原生实现**: `FDataLayerOutlinerIsVisibleColumn` (Private)

**功能描述**:
- 眼睛图标列，控制 DataLayer 的编辑器可见性
- 点击切换 `IsVisible` 状态
- 灰色眼睛表示父级不可见（继承状态）

**关键 API**:
```cpp
DataLayerInstance->SetIsVisible(bool bNewValue);
DataLayerInstance->IsVisible();
DataLayerInstance->IsEffectiveVisible(); // 考虑父级状态
```

**实现要点**:
- 继承 `FSceneOutlinerGutter` 基类（用于左侧小列）
- 使用 `SVisibilityWidget` 或自定义控件
- 支持多选切换

#### 2. IsLoadedInEditor 列（勾选框）
**原生实现**: `FDataLayerOutlinerIsLoadedInEditorColumn` (Private)

**功能描述**:
- 勾选框列，控制 DataLayer 是否在编辑器中加载
- 影响该 DataLayer 下所有 Actor 的加载状态
- 支持多选批量切换

**关键 API**:
```cpp
UDataLayerEditorSubsystem::SetDataLayerIsLoadedInEditor(DataLayer, bIsLoaded, bIsFromUserChange);
DataLayerInstance->IsEffectiveLoadedInEditor();
```

**实现要点**:
- 使用 `SCheckBox` 控件
- 绑定 `IsChecked` 和 `OnCheckStateChanged`
- 包裹在 `FScopedTransaction` 中支持撤销

---

### 中优先级（推荐实现）

#### 3. 双击设置 Current DataLayer
**原生行为**: 双击 DataLayer 行将其设置为 Actor Editor Context

**功能描述**:
- 双击后，新创建的 Actor 自动添加到该 DataLayer
- 按住 Ctrl 双击可切换而不清除其他 Context

**关键 API**:
```cpp
UDataLayerEditorSubsystem::AddToActorEditorContext(DataLayer);
UDataLayerEditorSubsystem::RemoveFromActorEditorContext(DataLayer);
```

**实现位置**: `FStageDataLayerMode::OnItemDoubleClick`

#### 4. Ctrl+B 定位 Content Browser
**功能描述**:
- 选中 DataLayer 后按 Ctrl+B 在 Content Browser 中定位对应的 DataLayerAsset

**关键 API**:
```cpp
GEditor->SyncBrowserToObjects(TArray<UObject*>{DataLayerAsset});
// 或
FContentBrowserModule::Get().SyncBrowserToAssets(Assets);
```

**实现位置**: `FStageDataLayerMode::OnKeyDown`

#### 5. 右键菜单扩展

需要在现有右键菜单中添加：

| 菜单项 | 功能 | API |
|--------|------|-----|
| Rename | 重命名 DataLayer | `SceneOutliner->RequestRename(Item)` |
| Toggle Loaded in Editor | 切换加载状态 | `SetDataLayerIsLoadedInEditor` |
| Find in Content Browser | 定位到 Asset | `SyncBrowserToAssets` |

**实现位置**: `FStageDataLayerMode::RegisterContextMenu`

---

### 低优先级（评估后决定）

#### 6. WorldDataLayers 根节点
**描述**: 显示 WorldDataLayers Actor 作为树的根节点

**原生实现**: `FWorldDataLayersTreeItem`

**评估点**:
- 是否需要显示根节点？
- 是否需要区分不同 World 的 DataLayers？

#### 7. 拖放设置父子关系
**描述**: 拖动 DataLayer 到另一个 DataLayer 上设置父子关系

**原生实现**: `FDataLayerMode::CreateDragDropOperation` + `OnDrop`

**评估点**:
- StageEditor 是否需要此功能？
- 父子关系对 Stage-Act 映射有何影响？

#### 8. 筛选选项
**描述**:
- Only Selected Actors: 只显示选中 Actor 所属的 DataLayer
- Highlight Selected: 高亮选中 Actor 所属的 DataLayer

**原生实现**: Mode 的 `bShowFilterOptions` + `HighlightSelectedDataLayers`

---

## 已排除功能

| 功能 | 排除原因 |
|------|----------|
| Delete 键删除 | StageEditor 不需要从此处删除 DataLayer |
| Make Current DataLayer 按钮 | 双击实现同等功能 |
| 层级删除按钮列 | 不需要删除功能 |
| Create DataLayer 按钮 | StageEditor 通过 Import 创建 |

---

## 技术参考

### 原生代码位置
```
Engine/Source/Editor/DataLayerEditor/Private/
├── DataLayerOutlinerIsVisibleColumn.h/cpp      # IsVisible 列
├── DataLayerOutlinerIsLoadedInEditorColumn.h/cpp # IsLoadedInEditor 列
├── DataLayerMode.h/cpp                         # Mode 实现
├── DataLayerHierarchy.h/cpp                    # Hierarchy 实现
└── WorldDataLayersTreeItem.h/cpp              # 根节点类型
```

### SceneOutliner 框架关键类
```cpp
// 列基类
class ISceneOutlinerColumn;
class FSceneOutlinerGutter;  // 用于小图标列（如可见性）

// 控件
class SVisibilityWidget;     // 可见性眼睛图标
class SCheckBox;             // 勾选框
```

---

## 实现计划

### Step 1: 实现 IsVisible 和 IsLoadedInEditor 列
1. 创建 `FStageDataLayerVisibilityColumn` 继承 `FSceneOutlinerGutter`
2. 创建 `FStageDataLayerLoadedColumn` 继承 `ISceneOutlinerColumn`
3. 在 `SStageDataLayerOutliner::InitializeOutliner` 中注册新列

### Step 2: 实现交互功能
1. 在 `FStageDataLayerMode::OnItemDoubleClick` 中实现 Current DataLayer 设置
2. 在 `FStageDataLayerMode::OnKeyDown` 中处理 Ctrl+B
3. 扩展 `RegisterContextMenu` 添加新菜单项

### Step 3: 评估并实现低优先级功能
1. 评估 WorldDataLayers 根节点需求
2. 评估拖放功能需求
3. 评估筛选选项需求

---

## 文件变更清单

| 文件 | 变更类型 | 说明 |
|------|----------|------|
| `StageDataLayerColumns.h` | 修改 | 添加 Visibility 和 Loaded 列声明 |
| `StageDataLayerColumns.cpp` | 修改 | 添加列实现 |
| `StageDataLayerMode.cpp` | 修改 | 添加双击、快捷键、右键菜单 |
| `SStageDataLayerOutliner.cpp` | 修改 | 注册新列 |

---

## 进度跟踪

| 任务 | 状态 | 备注 |
|------|------|------|
| 文档创建 | ✅ 完成 | 本文档 |
| IsVisible 列 | ✅ 完成 | `FStageDataLayerVisibilityColumn` + `SStageDataLayerVisibilityWidget` |
| IsLoadedInEditor 列 | ✅ 完成 | `FStageDataLayerLoadedInEditorColumn` |
| 双击交互 | ❌ 已移除 | 用户反馈：功能不实用，已移除 |
| Ctrl+B 快捷键 | ✅ 完成 | `OnKeyDown` + `FindInContentBrowser()` |
| F2 快捷键 | ✅ 完成 | 重命名功能 |
| F5 快捷键 | ✅ 完成 | 强制刷新 |
| 右键菜单扩展 | ✅ 完成 | Toggle Loaded, Load, Unload, Find in CB, Rename |
| 列宽度调整 | ✅ 完成 | Actions/SUID 列可手动调整宽度 |
| 关卡切换同步 | ✅ 完成 | `FEditorDelegates::MapChange` 事件订阅 |
| SUID 列 | ✅ 完成 | `FStageDataLayerSUIDColumn` - 显示 S:X 或 A:X.Y |
| **Std Rename 按钮** | ✅ 完成 | Actions 列 `Std Rename` 按钮 + `SStdRenamePopup` 精简对话框 |
| **Import 按钮改进** | ✅ 完成 | 始终显示 + 不规范命名时弹出警告确认 |
| 低优先级评估 | 🔲 待开始 | WorldDataLayers 根节点、拖放、筛选选项 |

---

## 实现详情

### 新增文件修改

| 文件 | 修改内容 |
|------|----------|
| `StageDataLayerColumns.h` | 新增 `FStageDataLayerVisibilityColumn`, `FStageDataLayerLoadedInEditorColumn`, `FStageDataLayerSUIDColumn` |
| `StageDataLayerColumns.cpp` | 新增三列实现 + `SStageDataLayerVisibilityWidget` + Actions 列 Rename 按钮 |
| `StageDataLayerMode.h` | 新增 `FindInContentBrowser()` 方法声明 |
| `StageDataLayerMode.cpp` | 完整实现快捷键、扩展右键菜单 |
| `SStageDataLayerOutliner.h` | 新增 `OnMapChanged()` 方法和 `OnMapChangedHandle` |
| `SStageDataLayerOutliner.cpp` | 注册新列、订阅 `MapChange` 事件、关卡切换处理 |
| `SStdRenamePopup.h/cpp` | **新增** 标准重命名精简对话框（替代旧的 `SDataLayerQuickRenameDialog`） |

### 列顺序 (从左到右)

1. Visibility (眼睛图标) - Priority 0
2. LoadedInEditor (勾选框) - Priority 1
3. SyncStatus (同步状态) - Priority 2
4. Label (名称) - Priority 3
5. SUID (Stage Unique ID) - Priority 4
6. Actions (操作按钮) - Priority 5

### 快捷键

| 按键 | 功能 |
|------|------|
| F2 | 重命名选中项 |
| F5 | 强制刷新树 |
| Ctrl+B | 在 Content Browser 中定位 |

### 列宽度设置

| 列 | 宽度类型 | 初始宽度 | 说明 |
|---|---|---|---|
| Visibility | Gutter (固定) | - | 眼睛图标，继承自 FSceneOutlinerGutter |
| LoadedInEditor | `FixedWidth` | 24px | 勾选框，固定宽度 |
| SyncStatus | `FixedWidth` | 24px | 同步状态图标，固定宽度 |
| Label | `FillWidth` | 5.0 | 内置列，自动填充剩余空间 |
| SUID | `ManualWidth` | 60px | **可手动拖拽调整宽度** |
| Actions | `ManualWidth` | 150px | **可手动拖拽调整宽度** |

---

## 已移除功能

| 功能 | 移除原因 |
|------|----------|
| 双击设置 Current DataLayer | 用户反馈：功能不实用 |

---

## 关卡切换同步

### 问题
DataLayer 列表不会随关卡切换而更新，仍显示旧关卡的 DataLayers。

### 解决方案
订阅 `FEditorDelegates::MapChange` 事件，在关卡切换时：
1. 更新 `RepresentingWorld` 为新世界
2. 清空缓存的选择（旧 DataLayer 引用）
3. 重新初始化 SceneOutliner
4. 重建 UI

### 实现
```cpp
// SStageDataLayerOutliner.cpp
void SStageDataLayerOutliner::SubscribeToEvents()
{
    // ... existing subscriptions ...
    OnMapChangedHandle = FEditorDelegates::MapChange.AddSP(
        this, &SStageDataLayerOutliner::OnMapChanged);
}

void SStageDataLayerOutliner::OnMapChanged(uint32 MapChangeFlags)
{
    UWorld* NewWorld = GEditor->GetEditorWorldContext().World();
    if (NewWorld != RepresentingWorld.Get())
    {
        RepresentingWorld = NewWorld;
        SelectedDataLayersSet.Empty();
        InitializeOutliner();
        // Rebuild UI...
    }
}
```

---

## Std Rename 功能（标准重命名）

### 功能描述
为降低用户重命名 DataLayer 的工作量，在 Actions 列添加 `Std Rename` 按钮，点击后弹出精简对话框。

### 设计理念
**DataLayer 层级决定身份，无例外：**
- **无父 DataLayer** → 一定是 Stage → 预填 `DL_Stage_`
- **有父 DataLayer** → 一定是 Act → 预填 `DL_Act_`

### 对话框功能
- 精简 UI：只有输入框 + OK/Cancel
- 自动判断类型并预填前缀
- 用户只需在前缀后补充名称
- Enter 键或 OK 按钮确认

### 实现文件
- `SStdRenamePopup.h/cpp` - 精简对话框 UI 和逻辑
- `StageDataLayerColumns.cpp` - Actions 列的 Std Rename 按钮

### 技术要点
1. `SStdRenamePopup::IsStageDataLayer()` - 通过 `Instance->GetParent()` 判断层级
2. 使用 `FAssetToolsModule::RenameAssets()` 执行重命名
3. 重命名后广播 `OnDataLayerChanged` 事件刷新 UI

---

## Import 按钮改进

### 变更内容
- **始终显示**：NotImported 状态下始终显示 Import 按钮（不再检查命名规范）
- **警告机制**：如果命名不符合规范，弹出警告对话框让用户确认

### 警告对话框内容
```
The DataLayer "{Name}" does not follow the naming convention.

Expected formats:
  - Stage: DL_Stage_<Name>
  - Act: DL_Act_<Name>

Do you want to import it anyway?
```

### 技术实现
使用 `FMessageDialog::Open(EAppMsgType::YesNo, ...)` 弹出确认框

---

## Actions 列按钮布局

| 按钮 | 显示条件 | 说明 |
|------|----------|------|
| **Std Rename** | 始终显示 | 标准重命名（根据层级自动判断 Stage/Act） |
| **Import** | NotImported 状态 | 导入到 StageEditor（不规范命名时警告） |
| **Sync** | OutOfSync 状态 | 同步变更 |

---

*文档创建: 2025-11-29 22:30*
*最后更新: 2025-11-30 12:00*
