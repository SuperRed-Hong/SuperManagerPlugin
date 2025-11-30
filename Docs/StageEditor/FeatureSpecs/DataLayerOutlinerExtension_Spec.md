# DataLayer Outliner 扩展功能规格文档

> 创建日期: 2025-11-29
> 状态: 需求确认完成，待实施
> 关联文档: DataLayer_ReverseSync_Design.md

## 1. 功能概述

### 1.1 背景与目标

用户在未安装 StageEditor 插件时，可能已经通过手动方式布设了 DataLayer 结构。本功能允许 StageEditor 插件根据已有的 DataLayer 数据（名称、层级、成员 Actor），通过智能匹配算法自动创建 Stage-Act-Prop 架构，实现**项目中途接管**能力。

### 1.2 核心价值

- **降低迁移成本**：无需重建 DataLayer，直接复用现有结构
- **提高工作效率**：一键导入，自动识别层级关系
- **保持数据同步**：实时监测 DataLayer 变化，提示用户同步

---

## 2. DataLayer 命名规范

### 2.1 标准命名格式

```
Stage DataLayer:  DL_Stage_{StageName}
Act DataLayer:    DL_Act_{StageName}_{ActName}
```

**示例：**
```
DL_Stage_Castle
├── DL_Act_Castle_Exterior
├── DL_Act_Castle_GreatHall
└── DL_Act_Castle_Dungeon
```

### 2.2 命名规则

| 规则 | 说明 |
|------|------|
| Stage 前缀 | `DL_Stage_` |
| Act 前缀 | `DL_Act_` |
| 不包含数字 ID | SUID 由系统内部管理，命名纯语义化 |
| Act 必须为 Stage 子层 | 父子层级关系强制要求 |

### 2.3 快速重命名辅助

为降低用户重命名工作量，提供快速重命名按钮：
- **Rename → Stage**：用户只需输入 `{StageName}`，自动生成 `DL_Stage_{StageName}`
- **Rename → Act**：用户只需输入 `{ActName}`，自动生成 `DL_Act_{ParentStageName}_{ActName}`

---

## 3. DataLayerOutliner UI 扩展

### 3.1 列布局

在现有 DataLayerOutliner 的 MultiColumn TreeView 中新增以下列：

```
┌─────────────┬──────────────────────┬────────────┬──────────────┬─────────────┐
│ Sync Status │ Item Label (原有)    │ SUID       │ Actions      │ ... (原有列) │
├─────────────┼──────────────────────┼────────────┼──────────────┼─────────────┤
│ (绿色图标)  │ DL_Stage_Castle      │ S:1        │ [Rename][...│             │
│ ⚠ (橙色)    │ DL_Act_Castle_Ext    │ A:1.2      │ [Rename][Syn│             │
│ ● (蓝色)    │ DL_Other_Random      │            │ [Rename][Imp│             │
└─────────────┴──────────────────────┴────────────┴──────────────┴─────────────┘
```

**列顺序（从左到右）：**
1. **Sync Status** - 同步状态图标（新增，在 Item Label 之前）
2. **Item Label** - 原有 DataLayer 名称列
3. **SUID** - Stage/Act 唯一标识符（新增，在 Item Label 之后）
4. **Actions** - 操作按钮汇总列（新增）
5. 其他原有列...

### 3.2 Sync Status 列行为

| 状态 | 图标 | 显示行为 | 说明 |
|------|------|---------|------|
| **已同步** | ✓ 绿色 | Hover 时显示，不 Hover 时隐藏 | 类似 SceneOutliner 可见性图标 |
| **不同步** | ⚠ 橙色 | 始终显示 | 提示用户需要同步 |
| **未导入** | ● 蓝色 | 始终显示 | 提示用户可以导入 |

**Hover 提示内容（不同步状态）：**

| 不同步原因 | 提示文案 (中文) | 提示文案 (英文) |
|-----------|----------------|----------------|
| 新增 Act DataLayer | `"检测到新 Act: {名称}，点击同步以导入"` | `"New Act detected: {Name}. Click Sync to import."` |
| 删除 Act DataLayer | `"Act '{名称}' 已被删除，点击同步以更新 Stage"` | `"Act '{Name}' was removed. Click Sync to update Stage."` |
| Actor 成员变化 | `"Props 变化：新增 {N} 个，移除 {M} 个，点击同步以更新"` | `"Props changed: {N} added, {M} removed. Click Sync to update."` |

多种原因时，逐行显示所有原因。

### 3.3 SUID 列格式

| 类型 | 显示格式 | 示例 |
|------|---------|------|
| Stage | `S:{StageID}` | `S:1` |
| Act | `A:{StageID}.{ActID}` | `A:1.2` |
| 未导入 | 空 | |

### 3.4 Actions 列按钮

所有行内操作按钮汇总在 Actions 列中：

| 按钮 | 显示条件 | 功能 |
|------|---------|------|
| **Rename** | 常驻显示 | 快速重命名（弹出输入框） |
| **Import** | 符合 `DL_Stage_*` 命名 且 未导入 | 打开导入预览对话框 |
| **Sync** | 已导入 且 数据不同步 | 同步当前项 |

**按钮布局规则：**
- 常驻显示最多 2 个最相关按钮
- 更多选项通过下拉菜单收纳

### 3.5 工具栏扩展

在 DataLayerOutliner 工具栏**最左侧**添加：

| 按钮 | 功能 |
|------|------|
| **Sync All** | 批量同步所有标记为不同步的项 |

---

## 4. 导入预览对话框

### 4.1 触发方式

点击 Actions 列的 **Import** 按钮

### 4.2 对话框布局

```
┌─────────────────────────────────────────────────────────────┐
│ Import DataLayer as Stage                                   │
├─────────────────────────────────────────────────────────────┤
│ Source: DL_Stage_Castle                                     │
│                                                             │
│ Will Create:                                                │
│ ┌───────────────────────────────────────┬──────────────────┐│
│ │ Item Label                            │ SUID             ││
│ ├───────────────────────────────────────┼──────────────────┤│
│ │ ✓ Stage: Castle                       │ S:1              ││
│ │   ├── Act: Exterior                   │ A:1.1            ││
│ │   │   └── Props: 12 actors            │                  ││
│ │   ├── Act: GreatHall                  │ A:1.2            ││
│ │   │   └── Props: 8 actors             │                  ││
│ │   └── Act: Dungeon                    │ A:1.3            ││
│ │       └── Props: 15 actors            │                  ││
│ └───────────────────────────────────────┴──────────────────┘│
│                                                             │
│ Total: 1 Stage, 3 Acts, 35 Props                            │
│                                                             │
│                              [Cancel]  [Import]             │
└─────────────────────────────────────────────────────────────┘
```

**特点：**
- MultiColumn TreeView 展示（Item Label + SUID 两列）
- 显示将要创建的完整层级结构
- 显示每个 Act 下将注册的 Props 数量
- 底部汇总统计

---

## 5. 元数据存储设计

### 5.1 UAssetUserData 类定义

```cpp
UCLASS()
class STAGEEDITOR_API UStageEditorDataLayerUserData : public UAssetUserData
{
    GENERATED_BODY()

public:
    /** 关联的 Stage/Act SUID */
    UPROPERTY(VisibleAnywhere)
    FSUID LinkedSUID;

    /** 导入/同步时的时间戳 */
    UPROPERTY(VisibleAnywhere)
    FDateTime LastSyncTime;

    /** 导入时子 DataLayer 的名称列表（仅 Stage 级别使用） */
    UPROPERTY(VisibleAnywhere)
    TArray<FString> LastKnownChildDataLayerNames;

    /** 导入时包含的 Actor 软引用列表 */
    UPROPERTY(VisibleAnywhere)
    TArray<TSoftObjectPtr<AActor>> LastKnownActors;
};
```

### 5.2 元数据附加方式

```cpp
// 附加元数据到 DataLayerAsset
void AttachStageEditorUserData(UDataLayerAsset* Asset, const FSUID& SUID)
{
    auto* UserData = NewObject<UStageEditorDataLayerUserData>(Asset);
    UserData->LinkedSUID = SUID;
    UserData->LastSyncTime = FDateTime::Now();
    UserData->LastKnownChildDataLayerNames = GetChildDataLayerNames(Asset);
    UserData->LastKnownActors = GetActorsInDataLayer(Asset);

    Asset->AddAssetUserData(UserData);
}

// 读取元数据
UStageEditorDataLayerUserData* GetStageEditorUserData(UDataLayerAsset* Asset)
{
    return Asset->GetAssetUserData<UStageEditorDataLayerUserData>();
}
```

### 5.3 同步状态检测逻辑

```cpp
UENUM()
enum class ESyncStatus : uint8
{
    NotImported,    // 未导入（蓝色）
    Synced,         // 已同步（绿色）
    OutOfSync       // 不同步（橙色）
};

struct FSyncStatusInfo
{
    ESyncStatus Status;
    TArray<FString> NewChildDataLayers;      // 新增的子层名称
    TArray<FString> RemovedChildDataLayers;  // 被删除的子层名称
    int32 AddedActorCount;                   // 新增 Actor 数量
    int32 RemovedActorCount;                 // 移除 Actor 数量
};

FSyncStatusInfo GetSyncStatusInfo(UDataLayerAsset* Asset)
{
    FSyncStatusInfo Info;
    auto* UserData = Asset->GetAssetUserData<UStageEditorDataLayerUserData>();

    // 未导入
    if (!UserData || !UserData->LinkedSUID.IsValid())
    {
        Info.Status = ESyncStatus::NotImported;
        return Info;
    }

    // 检查子层变化
    TArray<FString> CurrentChildNames = GetChildDataLayerNames(Asset);
    for (const FString& Name : CurrentChildNames)
    {
        if (!UserData->LastKnownChildDataLayerNames.Contains(Name))
            Info.NewChildDataLayers.Add(Name);
    }
    for (const FString& Name : UserData->LastKnownChildDataLayerNames)
    {
        if (!CurrentChildNames.Contains(Name))
            Info.RemovedChildDataLayers.Add(Name);
    }

    // 检查 Actor 变化
    TArray<TSoftObjectPtr<AActor>> CurrentActors = GetActorsInDataLayer(Asset);
    for (const auto& Actor : CurrentActors)
    {
        if (!UserData->LastKnownActors.Contains(Actor))
            Info.AddedActorCount++;
    }
    for (const auto& Actor : UserData->LastKnownActors)
    {
        if (!CurrentActors.Contains(Actor))
            Info.RemovedActorCount++;
    }

    // 判断状态
    if (Info.NewChildDataLayers.Num() > 0 ||
        Info.RemovedChildDataLayers.Num() > 0 ||
        Info.AddedActorCount > 0 ||
        Info.RemovedActorCount > 0)
    {
        Info.Status = ESyncStatus::OutOfSync;
    }
    else
    {
        Info.Status = ESyncStatus::Synced;
    }

    return Info;
}
```

### 5.4 Hover 提示生成

```cpp
FText GenerateSyncTooltip(const FSyncStatusInfo& Info)
{
    TArray<FText> Lines;

    for (const FString& Name : Info.NewChildDataLayers)
    {
        Lines.Add(FText::Format(
            LOCTEXT("NewActDetected", "检测到新 Act: {0}，点击同步以导入"),
            FText::FromString(Name)));
    }

    for (const FString& Name : Info.RemovedChildDataLayers)
    {
        Lines.Add(FText::Format(
            LOCTEXT("ActRemoved", "Act '{0}' 已被删除，点击同步以更新 Stage"),
            FText::FromString(Name)));
    }

    if (Info.AddedActorCount > 0 || Info.RemovedActorCount > 0)
    {
        Lines.Add(FText::Format(
            LOCTEXT("PropsChanged", "Props 变化：新增 {0} 个，移除 {1} 个，点击同步以更新"),
            FText::AsNumber(Info.AddedActorCount),
            FText::AsNumber(Info.RemovedActorCount)));
    }

    return FText::Join(FText::FromString(TEXT("\n")), Lines);
}
```

---

## 6. 不同步触发条件

| 条件 | 触发不同步 | 说明 |
|------|-----------|------|
| 新增 Act DataLayer | ✓ | 用户新建了符合命名规范的子 DataLayer |
| 删除 Act DataLayer | ✓ | 用户删除了已导入的 Act DataLayer |
| Actor 成员变化 | ✓ | DataLayer 中新增或移除了 Actor |
| DataLayer 重命名 | ✗ | 不影响，Stage 通过对象引用而非名称关联 |

---

## 7. 本地化支持

### 7.1 技术方案

使用 UE 标准本地化机制 `LOCTEXT` / `NSLOCTEXT`：

```cpp
#define LOCTEXT_NAMESPACE "StageEditorDataLayerSync"

// 所有用户可见文本使用 LOCTEXT
LOCTEXT("NewActDetected", "检测到新 Act: {0}，点击同步以导入")
LOCTEXT("SyncAll", "Sync All")
LOCTEXT("ImportAsStage", "Import as Stage")

#undef LOCTEXT_NAMESPACE
```

### 7.2 翻译文件

需要创建翻译配置：
- `Config/Localization/StageEditor.ini`
- `Content/Localization/StageEditor/en/StageEditor.po`
- `Content/Localization/StageEditor/zh-Hans/StageEditor.po`

---

## 8. 增量导入规则

### 8.1 判断已导入

通过 `UAssetUserData` 元数据判断：
- 存在 `UStageEditorDataLayerUserData` 且 `LinkedSUID.IsValid()` → 已导入
- 否则 → 未导入

### 8.2 增量导入入口

通过 DataLayerOutliner 的 Sync Status 列和 Actions 列：
- 蓝色图标（未导入）+ Import 按钮 → 可导入
- 橙色图标（不同步）+ Sync 按钮 → 需同步

### 8.3 防重复导入

导入时检查 `UStageEditorDataLayerUserData`，如已存在则阻止并提示用户。

---

## 9. 实施计划

### Phase 1: 元数据基础设施
- [ ] 创建 `UStageEditorDataLayerUserData` 类
- [ ] 实现元数据附加/读取 API
- [ ] 实现 `ESyncStatus` 检测逻辑
- [ ] 实现 `FSyncStatusInfo` 和 Tooltip 生成

### Phase 2: DataLayerOutliner 列扩展
- [ ] 研究 DataLayerOutliner 扩展机制
- [ ] 添加 Sync Status 列（图标 + Hover 逻辑）
- [ ] 添加 SUID 列
- [ ] 添加 Actions 列（按钮容器）

### Phase 3: Actions 列按钮
- [ ] Rename 按钮（快速重命名弹窗）
- [ ] Import 按钮（触发导入预览对话框）
- [ ] Sync 按钮（同步单项）

### Phase 4: 导入预览对话框
- [ ] 对话框 UI 布局（MultiColumn TreeView）
- [ ] 层级扫描与预览数据生成
- [ ] Import 执行逻辑

### Phase 5: 工具栏扩展
- [ ] Sync All 按钮
- [ ] 批量同步逻辑

### Phase 6: 本地化
- [ ] 提取所有用户可见文本到 LOCTEXT
- [ ] 配置本地化文件结构
- [ ] 中英文翻译

---

## 10. 待确认事项

1. **DataLayerOutliner 扩展机制**：需要进一步研究 UE 的 DataLayerOutliner 是否支持插件扩展列，或者需要用其他方式实现。

2. **Rename 弹窗细节**：具体的输入框样式和交互流程。

3. **Sync All 的范围**：是同步当前关卡的所有不同步项，还是整个项目的？

---

## 11. 确认的设计决策汇总

| 决策点 | 选择 | 原因 |
|--------|------|------|
| 命名规范 | `DL_Stage_{Name}` / `DL_Act_{Stage}_{Name}` | 语义清晰，便于智能匹配 |
| 数字 ID 在命名中 | 不包含 | SUID 由系统内部管理 |
| Act 层级要求 | 必须为 Stage 子层 | 确保层级关系正确 |
| 重命名保留原名 | 是 | SUID 已作为唯一标识 |
| 元数据存储方案 | UAssetUserData | 性能最优（O(1)查询） |
| 本地化支持 | 多语言 | 跟随编辑器语言设置 |
| Sync All 位置 | 工具栏最左侧 | 高可见性 |
| 按钮布局 | 常驻2个 + 下拉菜单 | 平衡可用性和整洁性 |

---

**文档版本：v1.0**
**最后更新：2025-11-29**
