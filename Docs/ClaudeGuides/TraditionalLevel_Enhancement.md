# 传统 Level 下的 StageEditor 增强方案

## 问题

在非 World Partition Level 中，`DataLayer` 只控制**可见性**，不控制**资源加载**。

## 解决方案：混合使用 Level Streaming

### 方案 1：一个 Act = 一个 Sub-Level

**架构：**
```
MainLevel (Persistent)
  ├── StageActor (常驻)
  └── Sub-Levels (动态加载)
       ├── Act0_Level (Default)
       ├── Act1_Level (白天场景)
       └── Act2_Level (夜晚场景)
```

**实现：**

```cpp
// AStage.h - 添加成员
UPROPERTY(EditAnywhere, Category = "Stage|Level Streaming")
TMap<int32, TSoftObjectPtr<UWorld>> ActLevelMap; // ActID -> Sub-Level

// AStage.cpp - 修改 ActivateAct
void AStage::ActivateAct(int32 ActID)
{
    // ... 现有的 PropState 逻辑 ...

    // 如果不是 World Partition，使用 Level Streaming
    if (!GetWorld()->GetWorldPartition())
    {
        HandleTraditionalLevelStreaming(ActID);
    }
    else
    {
        // 现有的 DataLayer 逻辑
        HandleDataLayerActivation(ActID);
    }
}

void AStage::HandleTraditionalLevelStreaming(int32 ActID)
{
    // 卸载当前 Act 的 Level
    if (CurrentActID >= 0 && ActLevelMap.Contains(CurrentActID))
    {
        if (TSoftObjectPtr<UWorld>* LevelPtr = ActLevelMap.Find(CurrentActID))
        {
            FLatentActionInfo LatentInfo;
            UGameplayStatics::UnloadStreamLevel(this, LevelPtr->ToSoftObjectPath().GetAssetName(), LatentInfo, false);
        }
    }

    // 加载新 Act 的 Level
    if (ActLevelMap.Contains(ActID))
    {
        if (TSoftObjectPtr<UWorld>* LevelPtr = ActLevelMap.Find(ActID))
        {
            FLatentActionInfo LatentInfo;
            UGameplayStatics::LoadStreamLevel(this, LevelPtr->ToSoftObjectPath().GetAssetName(), true, true, LatentInfo);
        }
    }
}

void AStage::HandleDataLayerActivation(int32 ActID)
{
    // 现有的 DataLayer 代码 (Stage.cpp:62-84)
    // ...
}
```

**优势：**
- ✅ 真正的资源加载/卸载
- ✅ 内存高效
- ✅ 艺术家可以独立编辑每个 Act 的 Sub-Level

**劣势：**
- ⚠️ 需要手动创建和配置 Sub-Levels
- ⚠️ 加载有延迟（异步）

---

### 方案 2：DataLayer + Level Streaming 混合

**适用场景：**
- 大部分 Props 共享（保持在主 Level）
- 少量特殊资源需要动态加载（使用 Level Streaming）

**架构：**
```
MainLevel
  ├── StageActor
  ├── Common Props (DataLayer 控制可见性)
  └── Sub-Levels (动态加载特殊资源)
       ├── Act1_SpecialEffects
       └── Act2_SpecialEffects
```

**实现：**
```cpp
// FAct 结构添加字段
USTRUCT()
struct FAct
{
    GENERATED_BODY()

    // 现有字段...
    UPROPERTY(EditAnywhere)
    TObjectPtr<UDataLayerAsset> AssociatedDataLayer;

    // 新增：可选的 Sub-Level 引用
    UPROPERTY(EditAnywhere, Category = "Level Streaming")
    TSoftObjectPtr<UWorld> OptionalSubLevel;
};

// AStage::ActivateAct 修改
void AStage::ActivateAct(int32 ActID)
{
    const FAct* TargetAct = Acts.FindByPredicate([ActID](const FAct& Act) {
        return Act.ActID.ActID == ActID;
    });

    if (!TargetAct) return;

    // 1. 应用 PropState (始终执行)
    ApplyPropStates(TargetAct);

    // 2. DataLayer 处理（可见性或加载，取决于 WP）
    if (GetWorld()->GetWorldPartition())
    {
        HandleDataLayerActivation(TargetAct);
    }
    else
    {
        HandleDataLayerVisibility(TargetAct); // 仅可见性
    }

    // 3. 如果有额外的 Sub-Level，加载它
    if (TargetAct->OptionalSubLevel.IsValid())
    {
        LoadSubLevel(TargetAct->OptionalSubLevel);
    }

    // 4. 卸载上一个 Act 的 Sub-Level
    UnloadPreviousSubLevel();
}
```

**优势：**
- ✅ 灵活：常用资源用 DataLayer，特殊资源用 Level Streaming
- ✅ 性能：大部分切换很快（DataLayer），少量加载有延迟（Streaming）

**劣势：**
- ⚠️ 配置稍复杂

---

### 方案 3：纯 DataLayer（仅可见性管理）

**适用场景：**
- 小型场景
- 所有资源可以一次性加载
- 只需要显示/隐藏切换

**优势：**
- ✅ 当前代码无需修改，直接可用
- ✅ 切换瞬间完成（无加载延迟）
- ✅ 简单易用

**劣势：**
- ❌ 所有资源始终占用内存
- ❌ 不适合大型场景

**使用建议：**
如果你的场景满足以下条件，直接使用现有代码即可：
- 场景资源总量 < 500MB
- 所有 Props 数量 < 1000 个
- 运行目标平台内存充足（桌面端/主机）

---

## 推荐实施步骤

### 阶段 1：保持现状（快速原型）
- ✅ 使用当前代码
- ✅ 在小型测试场景中验证功能
- ✅ 专注于核心玩法逻辑

### 阶段 2：性能优化（如需要）
根据实际需求选择：
- **小型项目** → 方案 3（纯 DataLayer）
- **中型项目** → 方案 2（混合模式）
- **大型项目** → 迁移到 World Partition

### 阶段 3：生产优化
- 迁移到 World Partition Level
- 享受完整的流式加载能力

---

## 编辑器提示

建议在 `FStageEditorController::Initialize()` 中添加提示：

```cpp
void FStageEditorController::Initialize()
{
    BindEditorDelegates();
    FindStageInWorld();

    // 检测并提示当前模式
    if (IsWorldPartitionActive())
    {
        DebugHeader::ShowNotifyInfo(
            TEXT("✅ World Partition: Full streaming support")
        );
    }
    else
    {
        DebugHeader::ShowNotifyInfo(
            TEXT("⚠️ Traditional Level: DataLayer controls visibility only.\n")
            TEXT("For resource streaming, consider:\n")
            TEXT("1. Convert to World Partition, OR\n")
            TEXT("2. Use Level Streaming for dynamic loading")
        );
    }
}
```

---

## 总结

| 方案 | 资源加载 | 配置复杂度 | 适用场景 |
|------|---------|-----------|---------|
| 方案 1 (Level Streaming) | ✅ 完整支持 | ⚠️ 中等 | 各 Act 资源差异大 |
| 方案 2 (混合) | ✅ 部分支持 | ⚠️ 中等 | 共享大部分资源 |
| 方案 3 (纯 DataLayer) | ❌ 不支持 | ✅ 简单 | 小型场景 |
| **World Partition** | ✅ 完整支持 | ✅ 简单 | **推荐** |

**最终建议：如果可能，转向 World Partition，这是 UE5 的未来方向。**
