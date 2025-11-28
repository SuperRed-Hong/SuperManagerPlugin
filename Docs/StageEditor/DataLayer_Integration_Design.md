# DataLayer 与 Stage/Act/Prop 集成方案设计文档

> 创建日期: 2025-11-22
> 状态: Phase 1-3 已实施

## 1. 问题分析

### 1.1 当前实现的问题

```cpp
// 当前代码 (StageEditorController.cpp:780-784)
FDataLayerCreationParameters Params;
Params.bIsPrivate = true;  // ❌ 私有DataLayer没有Asset
UDataLayerInstance* NewDataLayer = DataLayerSubsystem->CreateDataLayerInstance(Params);
```

- 使用`bIsPrivate = true`创建的是**私有DataLayer**，不关联`UDataLayerAsset`
- 但`FAct::AssociatedDataLayer`类型是`TObjectPtr<UDataLayerAsset>`，需要Asset引用
- 导致创建的DataLayer无法被Act引用

### 1.2 UE5 DataLayer架构

```
UDataLayerAsset (Content资产)
    ↓ 引用
UDataLayerInstance (关卡实例)
    ↓ 管理
AWorldDataLayers (关卡中的管理Actor)
    ↓ 包含
AActor (带DataLayerAsset引用)
```

**关键概念：**
- **UDataLayerAsset**: 持久化资产，存储在Content目录，可版本控制
- **UDataLayerInstance**: 关卡中的实例，引用DataLayerAsset，管理Actor的加载状态
- **AWorldDataLayers**: 关卡中管理所有DataLayerInstance的Actor

### 1.3 为什么不能使用Private DataLayer

**核心原因：Actor的DataLayer引用必须通过Asset**

```cpp
// AActor 中的 DataLayer 存储方式 (Engine源码)
UPROPERTY(EditAnywhere, Category = DataLayer)
TArray<TObjectPtr<UDataLayerAsset>> DataLayerAssets;  // ← 存储的是Asset引用！
```

**Private DataLayer的限制：**

| 特性 | Asset-based DataLayer | Private DataLayer |
|------|----------------------|-------------------|
| 有UDataLayerAsset | ✅ 有 | ❌ 无 |
| Actor可添加标签 | ✅ 可以 | ❌ 不可以 |
| Details面板可见 | ✅ 可见 | ❌ 不可见 |
| 持久化保存 | ✅ 随关卡保存 | ⚠️ 仅运行时 |
| 跨关卡复用 | ✅ 支持 | ❌ 不支持 |

**结论：** 如果使用Private DataLayer（`bIsPrivate = true`），Actor无法通过`DataLayerAssets`属性引用它，也就无法被该DataLayer管理。这就是当前实现失效的根本原因。

---

## 2. 方案选项

### 方案A: 基于Asset的DataLayer（推荐）

**流程：**
1. 为每个Act创建`UDataLayerAsset`资产（保存到Content目录）
2. 通过Asset在关卡中创建`UDataLayerInstance`
3. Prop添加到DataLayerInstance时，使用Asset引用

**优点：**
- 符合UE官方设计模式
- Asset可复用、可版本控制
- 支持跨关卡引用

**缺点：**
- 会产生较多Asset文件
- 需要管理Asset生命周期

---

### 方案B: 纯Private DataLayer

**流程：**
- 只使用私有DataLayer，不创建Asset
- 修改`FAct`结构，用名称/ID引用而非Asset指针

**优点：**
- 不产生额外Asset文件
- 实现简单

**缺点：**
- 无法持久化配置
- 不支持跨关卡
- 偏离UE官方最佳实践

---

### 方案C: 混合模式

**流程：**
- Stage根DataLayer使用Asset（可复用）
- Act子DataLayer使用Private（轻量）
- 通过名称关联而非Asset引用

---

## 3. 推荐方案：方案A（基于Asset）

### 3.1 资产目录结构

```
/Game/StageEditor/DataLayers/
├── DL_Stage_MainStage.uasset
├── DL_Act_MainStage_Act0_Default.uasset
├── DL_Act_MainStage_Act1_Combat.uasset
└── ...
```

### 3.2 数据结构修改

```cpp
// FAct 保持不变
UPROPERTY(EditAnywhere, Category = "Act")
TObjectPtr<UDataLayerAsset> AssociatedDataLayer;  // ✓ 已正确

// 可选：添加自动命名配置到Stage
UPROPERTY(EditAnywhere, Category = "Stage|DataLayer")
FDirectoryPath DataLayerAssetPath;  // 默认: /StageEditor/DataLayers/
```

### 3.3 核心API设计

```cpp
// StageEditorController.h
class FStageEditorController
{
public:
    //----------------------------------------------------------------
    // DataLayer Asset Management
    //----------------------------------------------------------------

    /** 创建DataLayerAsset资产并保存到Content */
    UDataLayerAsset* CreateDataLayerAsset(const FString& AssetName, const FString& FolderPath);

    /** 为Asset创建关卡中的DataLayerInstance */
    UDataLayerInstance* CreateDataLayerInstanceFromAsset(UDataLayerAsset* Asset);

    /** 获取Asset对应的Instance（如不存在则创建） */
    UDataLayerInstance* GetOrCreateInstanceForAsset(UDataLayerAsset* Asset);

    //----------------------------------------------------------------
    // Stage/Act DataLayer Integration
    //----------------------------------------------------------------

    /** 为Stage创建根DataLayer (Asset + Instance) */
    bool CreateDataLayerForStage(AStage* Stage);

    /** 为Act创建DataLayer (Asset + Instance)，作为Stage子层 */
    bool CreateDataLayerForAct(int32 ActID);

    /** 将Prop添加到Act的DataLayer */
    bool AddPropToActDataLayer(int32 PropID, int32 ActID);

    /** 从Act的DataLayer移除Prop */
    bool RemovePropFromActDataLayer(int32 PropID, int32 ActID);

    /** 同步所有Prop到对应Act的DataLayer */
    bool SyncAllPropsToDataLayers();
};
```

### 3.4 工作流程

```
用户操作                          系统行为
─────────────────────────────────────────────────────────
[Create Stage]
    └── CreateDataLayerForStage()
            ├── CreateDataLayerAsset("DL_Stage_XXX")
            ├── 保存Asset到 /StageEditor/DataLayers/
            └── CreateDataLayerInstanceFromAsset()

[Create Act]
    └── CreateDataLayerForAct(ActID)
            ├── CreateDataLayerAsset("DL_Act_XXX_ActN")
            ├── 保存Asset
            ├── CreateDataLayerInstanceFromAsset()
            ├── SetParent(StageDataLayerInstance)
            └── Act.AssociatedDataLayer = NewAsset

[Drag Prop to Act]
    └── AddPropToActDataLayer(PropID, ActID)
            ├── GetOrCreateInstanceForAsset(Act.AssociatedDataLayer)
            └── DataLayerSubsystem->AddActorToDataLayer(PropActor, Instance)

[Activate Act] (Runtime)
    └── Stage::ActivateAct(ActID)
            ├── Unload previous DataLayer
            └── Activate new DataLayer
```

### 3.5 需要解决的问题

| 问题 | 解决方案 |
|------|----------|
| Asset命名冲突 | 使用 Stage名+Act名+时间戳 |
| Asset删除时机 | Act删除时提示用户是否删除Asset |
| 已有Asset复用 | 提供"选择现有DataLayer"选项 |
| 跨关卡DataLayer | Asset可被多关卡的Instance引用 |
| Undo/Redo | FScopedTransaction + Asset操作事务 |

---

## 4. 实施计划

### Phase 1: 基础Asset管理 ✅
- [x] 实现`CreateDataLayerAsset()` - 创建Asset并保存到Content
- [x] 实现`GetOrCreateInstanceForAsset()` - 获取或创建Instance
- [x] 默认路径: `/StageEditor/DataLayers/`

### Phase 2: Stage/Act集成 ✅
- [x] 重写`CreateDataLayerForStage()` - 使用Asset方式
- [x] 重写`CreateDataLayerForAct()` - 创建Asset并关联到Act.AssociatedDataLayer
- [x] 父子层级关系设置

### Phase 3: Prop同步 ✅
- [x] 更新`AssignPropToStageDataLayer()` - 使用Asset查找Instance
- [x] 更新`RemovePropFromStageDataLayer()` - 使用Asset查找Instance
- [x] `AssignPropToActDataLayer()` - 已正确使用Asset

### Phase 4: UI集成
- [x] Act右键菜单：创建DataLayer（已有）
- [ ] 显示DataLayer关联状态
- [ ] DataLayer快速预览

### Phase 5: 清理与完善
- [ ] Asset删除逻辑
- [ ] Undo/Redo测试
- [ ] 错误处理与用户提示

---

## 5. 待确认问题

1. **DataLayer资产存放路径**：默认`/StageEditor/DataLayers/`是否合适？
2. **是否需要支持选择已有DataLayer**：还是只支持自动创建？
3. **Stage删除时**：是否自动删除关联的所有DataLayer Asset？
4. **命名规则**：`DL_Stage_XXX` / `DL_Act_XXX_ActN` 格式是否满足需求？

---

**请审阅此方案，确认后再开始实施。**
