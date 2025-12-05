# StageRegistry 持久化架构重设计 - 专家评审报告

> **评审日期**: 2025-12-04
> **文档版本**: Phase 13 设计完成版
> **专家小组**: Martin Fowler (架构), Michael Nygard (可靠性), Karl Wiegers (需求), Gojko Adzic (可测试性)

---

## 📊 整体评估

### 可实施性评分: **7.5/10** ✅ 基本可实施，需补充细节

| 维度 | 评分 | 说明 |
|------|------|------|
| **核心架构** | 9/10 ⭐⭐⭐⭐⭐ | 双层架构清晰，职责分离合理 |
| **技术细节** | 6/10 ⭐⭐⭐ | 部分关键细节模糊，需明确 |
| **错误处理** | 5/10 ⭐⭐ | 几乎没有考虑失败场景 |
| **实施计划** | 4/10 ⭐⭐ | 第 8 节标记"待确定" |
| **测试策略** | 3/10 ⭐ | 完全缺失 |

### 核心优势 ✅

1. **问题识别清晰** - "Stage 存在性 vs Actor 加载状态"概念区分准确
2. **架构设计合理** - DataAsset 持久化 + Subsystem 运行时的分层架构符合 UE 最佳实践
3. **职责分离明确** - Runtime 查询 + Editor 管理的双 Subsystem 架构优秀
4. **设计决策完整** - 4 个关键设计点都有详细讨论和方案对比

---

## 🔴 P0 - 必须解决（阻塞实施）

### 1. FLevelInstanceID 稳定性验证缺失

**Martin Fowler - 架构设计**:
> ❌ **严重问题**: 文档中提到 `FLevelInstanceID` 是"运行时计算的唯一 Hash"，但没有验证其在多次加载之间的稳定性。如果每次加载生成不同的 Hash，`RuntimeStageMap` 的 Key 就不稳定。

**具体问题**:
```cpp
// 文档中的设计
TMap<FStageRuntimeID, TWeakObjectPtr<AStage>> RuntimeStageMap;

// FStageRuntimeID 依赖 FLevelInstanceID
struct FStageRuntimeID {
    FLevelInstanceID LevelInstanceID;  // ← 这个 Hash 在重新加载时是否一致？
    int32 StageID;
};
```

**建议补充**:
```cpp
// 需要在文档中明确说明并验证：
1. FLevelInstanceID 是基于 Actor GUID 计算，GUID 持久化在 Level 文件中
2. 只要 LevelInstance Actor 不被删除重建，GUID 和 Hash 保持一致
3. 如果 LevelInstance Actor 被删除重建，需要什么迁移策略？

// 补充判空逻辑
bool IsMainLevel(const FLevelInstanceID& ID) const
{
    return ID.Hash == 0;  // ← 这个假设是否正确？需要查阅 UE 源码
}
```

**风险**: 🔥 高 - 如果 Hash 不稳定，整个 LevelInstance 支持方案崩溃

---

### 2. 事务一致性处理缺失

**Michael Nygard - 可靠性工程**:
> ❌ **严重问题**: CRUD 操作中 RegistryAsset 修改和 Level 修改不在同一个事务中，缺少失败回滚机制。

**失败场景示例**:
```cpp
// 场景 1: 创建 Stage 过程中的失败点
FStageEditorController::CreateNewStage()
{
    FScopedTransaction Transaction(...);

    // 步骤 1: 创建 Stage Actor ✅
    AStage* NewStage = SpawnStageActor();

    // 步骤 2: 调用 EditorSubsystem 注册
    UStageEditorSubsystem* EditorSub = GetEditorSubsystem();
    int32 NewStageID = EditorSub->RegisterStage(NewStage);

    // 步骤 3: 内部流程
    //   - 加载 RegistryAsset ✅
    //   - 分配 StageID ✅
    //   - Check Out RegistryAsset (多人模式) ⚠️ 可能失败
    //   - 添加到 StageEntries ✅
    //   - MarkPackageDirty() ✅
    //   - 添加到 RuntimeCache ✅

    // ❌ 问题：如果 Check Out 失败，Stage Actor 已创建但未注册
    // ❌ 问题：如果保存 Registry 时磁盘满，数据不一致
}
```

**建议补充**:
```cpp
// 1. 在 EditorSubsystem::RegisterStage() 中添加验证和回滚
int32 UStageEditorSubsystem::RegisterStage(AStage* Stage)
{
    // 预检查
    if (Registry->CollaborationMode == ECollaborationMode::Multi)
    {
        if (!CheckOutRegistryFile(Registry))
        {
            // ❌ Check Out 失败，返回错误码
            UE_LOG(LogStageEditor, Error,
                TEXT("Failed to check out Registry. Please ensure Source Control is available."));
            return 0;  // 0 = 注册失败
        }
    }

    // 分配 ID
    int32 NewStageID = Registry->AllocateAndRegister(Stage);

    // 验证保存成功
    if (!SaveRegistryAsset(Registry))
    {
        // ❌ 保存失败，回滚
        Registry->RemoveEntry(NewStageID);
        UE_LOG(LogStageEditor, Error, TEXT("Failed to save Registry."));
        return 0;
    }

    // 成功后才添加到运行时缓存
    RuntimeSub->AddStageToRuntimeCache(Stage);
    return NewStageID;
}

// 2. 在 Controller 层处理注册失败
void FStageEditorController::CreateNewStage()
{
    FScopedTransaction Transaction(...);
    AStage* NewStage = SpawnStageActor();

    int32 NewStageID = EditorSub->RegisterStage(NewStage);
    if (NewStageID == 0)
    {
        // 注册失败，删除刚创建的 Actor
        NewStage->Destroy();
        ShowError(TEXT("Failed to register Stage. Please check Source Control and disk space."));
        return;
    }
}
```

**风险**: 🔥 高 - 用户可能遇到"幽灵 Stage"（Actor 存在但未注册）或"孤儿记录"（Registry 有记录但 Actor 不存在）

---

### 3. 旧数据迁移方案不够详细

**Karl Wiegers - 需求工程**:
> ⚠️ **重要问题**: 文档 5.5 节提到"旧数据迁移"，但缺少关键细节：如何保证迁移完整性？如何验证？如果迁移失败怎么办？

**当前方案问题**:
```
用户点击 [Create Registry]
    ↓
扫描 Level 中已有的 Stage Actor
    ↓
将它们的信息填入 StageEntries
    ↓
根据最大 StageID 设置 NextStageID = MaxID + 1

❌ 问题 1: 如果扫描过程中漏掉某些 Stage 怎么办？
❌ 问题 2: 如果两个 Stage 有相同的 StageID 怎么办？
❌ 问题 3: 如何让用户确认迁移结果？
❌ 问题 4: 迁移后如何验证数据完整性？
```

**建议补充迁移方案**:
```cpp
// 迁移对话框设计
┌─────────────────────────────────────────────────────────────┐
│  迁移现有 Stage 到 Registry                                  │
├─────────────────────────────────────────────────────────────┤
│  检测到 15 个现有 Stage Actor:                                │
│                                                              │
│  ✅ Stage_ForestLevel (StageID: 1)                           │
│  ✅ Stage_CastleLevel (StageID: 2)                           │
│  ⚠️ Stage_TestLevel (StageID: 0) ← 未初始化，将分配新 ID     │
│  ❌ Stage_OldLevel (StageID: 1) ← ID 冲突！将重新分配        │
│  ...                                                         │
│                                                              │
│  检测到的问题:                                                │
│  • 1 个 Stage 的 StageID 未初始化                            │
│  • 1 个 Stage 的 StageID 冲突                                │
│                                                              │
│  [💾 生成迁移报告]  [✅ 继续迁移]  [❌ 取消]                  │
└─────────────────────────────────────────────────────────────┘

// 迁移后验证工具
UStageEditorSubsystem::ValidateRegistryIntegrity(UStageRegistryAsset* Registry)
{
    TArray<FString> Issues;

    // 验证 1: Registry 中的每个条目都能找到对应的 Actor
    for (const FStageRegistryEntry& Entry : Registry->StageEntries)
    {
        AStage* Stage = Entry.StageActor.LoadSynchronous();
        if (!Stage)
        {
            Issues.Add(FString::Printf(
                TEXT("Registry 记录了 StageID=%d，但 Actor 不存在"), Entry.StageID));
        }
    }

    // 验证 2: Level 中的每个 Stage Actor 都在 Registry 中
    TArray<AStage*> AllStages = FindAllStagesInLevel(Registry->OwningLevel.LoadSynchronous());
    for (AStage* Stage : AllStages)
    {
        if (!Registry->FindEntry(Stage->StageID))
        {
            Issues.Add(FString::Printf(
                TEXT("发现未注册的 Stage Actor: %s (StageID=%d)"),
                *Stage->GetName(), Stage->StageID));
        }
    }

    // 验证 3: 没有重复的 StageID
    TSet<int32> SeenIDs;
    for (const FStageRegistryEntry& Entry : Registry->StageEntries)
    {
        if (SeenIDs.Contains(Entry.StageID))
        {
            Issues.Add(FString::Printf(TEXT("重复的 StageID: %d"), Entry.StageID));
        }
        SeenIDs.Add(Entry.StageID);
    }

    return Issues;
}
```

**风险**: 🟠 中高 - 迁移失败可能导致旧项目无法使用新系统

---

## 🟠 P1 - 强烈建议（影响质量）

### 4. 多人协作并发冲突处理缺失

**Michael Nygard - 分布式系统**:
> ⚠️ **设计缺陷**: 文档假设 Source Control 是独占式锁（Perforce 模式），但现代 Git 使用乐观并发控制。需要明确 Git 场景下的冲突解决策略。

**Git 场景下的并发冲突**:
```
时间线：两个开发者同时工作

Dev A 分支:
10:00  Pull latest (NextStageID = 5)
10:05  创建 Stage_A，分配 ID = 5
10:10  Commit + Push

Dev B 分支:
10:00  Pull latest (NextStageID = 5)  ← 同样的起点
10:05  创建 Stage_B，分配 ID = 5     ← ID 冲突！
10:15  Pull (发现冲突)
10:20  合并冲突...如何解决？

Registry 文件冲突内容：
<<<<<<< HEAD (Dev A)
NextStageID: 6
StageEntries:
  - StageID: 5
    StageActor: /Game/Levels/Stage_A
=======
NextStageID: 6
StageEntries:
  - StageID: 5
    StageActor: /Game/Levels/Stage_B
>>>>>>> Dev B
```

**建议补充冲突解决机制**:
```cpp
// 1. 检测冲突的工具
UStageEditorSubsystem::DetectRegistryConflicts(UStageRegistryAsset* Registry)
{
    // 检查是否有 StageID 指向不同的 Actor
    TMap<int32, TArray<TSoftObjectPtr<AStage>>> IDToActors;

    for (const FStageRegistryEntry& Entry : Registry->StageEntries)
    {
        IDToActors.FindOrAdd(Entry.StageID).Add(Entry.StageActor);
    }

    TArray<int32> ConflictingIDs;
    for (const auto& Pair : IDToActors)
    {
        if (Pair.Value.Num() > 1)
        {
            ConflictingIDs.Add(Pair.Key);
        }
    }

    return ConflictingIDs;
}

// 2. 冲突解决对话框
┌─────────────────────────────────────────────────────────────┐
│  Registry 合并冲突                                           │
├─────────────────────────────────────────────────────────────┤
│  检测到 1 个 StageID 冲突:                                    │
│                                                              │
│  StageID 5 被分配给了两个不同的 Stage:                        │
│  • /Game/Levels/Stage_A (您的版本)                           │
│  • /Game/Levels/Stage_B (远程版本)                           │
│                                                              │
│  解决方案:                                                    │
│  ◉ 为 Stage_B 重新分配新 ID (推荐)                           │
│     → Stage_A 保持 ID=5, Stage_B 改为 ID=6                   │
│                                                              │
│  ○ 保留两者，手动选择                                         │
│     → 您需要手动决定哪个 Stage 使用 ID=5                     │
│                                                              │
│  [🔧 自动解决]  [✋ 手动处理]  [❌ 取消合并]                  │
└─────────────────────────────────────────────────────────────┘

// 3. 自动冲突解决逻辑
UStageEditorSubsystem::AutoResolveConflicts(UStageRegistryAsset* Registry)
{
    TArray<int32> ConflictingIDs = DetectRegistryConflicts(Registry);

    for (int32 ConflictID : ConflictingIDs)
    {
        // 找到所有使用此 ID 的条目
        TArray<FStageRegistryEntry*> Conflicts;
        for (FStageRegistryEntry& Entry : Registry->StageEntries)
        {
            if (Entry.StageID == ConflictID)
            {
                Conflicts.Add(&Entry);
            }
        }

        // 保留第一个，其余重新分配 ID
        for (int32 i = 1; i < Conflicts.Num(); ++i)
        {
            int32 NewID = Registry->AllocateNewID();
            Conflicts[i]->StageID = NewID;

            // 更新 Actor 的 StageID
            AStage* Stage = Conflicts[i]->StageActor.LoadSynchronous();
            if (Stage)
            {
                Stage->StageID = NewID;
                Stage->MarkPackageDirty();
            }

            UE_LOG(LogStageEditor, Warning,
                TEXT("Resolved conflict: Stage %s ID changed from %d to %d"),
                *Stage->GetName(), ConflictID, NewID);
        }
    }
}
```

**风险**: 🟠 中高 - Git 团队会遇到 ID 冲突，缺少工具会导致数据损坏

---

### 5. EditorSubsystem 生命周期问题

**Martin Fowler - 架构设计**:
> ⚠️ **架构问题**: `UEditorSubsystem` 是全局单例，但管理的是 per-World 的 Registry。文档没有说明如何处理多 World 编辑场景。

**多 World 场景示例**:
```cpp
// UE 编辑器可以同时打开多个 Level
World_Forest.umap  → 需要 Forest_StageRegistry.uasset
World_Castle.umap  → 需要 Castle_StageRegistry.uasset

// EditorSubsystem 是单例
UStageEditorSubsystem (全局单例)
  ├── LoadedRegistries: TMap<FSoftObjectPath, UStageRegistryAsset*>
  │    ├── Forest_StageRegistry → UStageRegistryAsset*
  │    └── Castle_StageRegistry → UStageRegistryAsset*
  │
  └── 问题：HandleStageLoaded(AStage* Stage) 被触发时
             如何知道这个 Stage 属于哪个 World？
             如何找到对应的 RegistryAsset？
```

**建议补充**:
```cpp
// 1. 修改 HandleStageLoaded 逻辑
void UStageEditorSubsystem::HandleStageLoaded(AStage* Stage)
{
    UWorld* StageWorld = Stage->GetWorld();
    if (!StageWorld)
    {
        UE_LOG(LogStageEditor, Warning, TEXT("Stage has no World"));
        return;
    }

    // 根据 Stage 的 World 查找对应的 RegistryAsset
    UStageRegistryAsset* Registry = GetOrLoadRegistryAsset(StageWorld);
    if (!Registry)
    {
        UE_LOG(LogStageEditor, Warning,
            TEXT("No Registry for World: %s"), *StageWorld->GetName());
        return;
    }

    // 检查是否已注册
    if (Stage->StageID != 0)
    {
        // 已注册，添加到运行时缓存
        UStageManagerSubsystem* RuntimeSub =
            StageWorld->GetSubsystem<UStageManagerSubsystem>();
        RuntimeSub->AddStageToRuntimeCache(Stage);
    }
    else
    {
        // 未注册（旧数据），执行注册
        RegisterStageInternal(Registry, Stage);
    }
}

// 2. 补充缓存清理逻辑
void UStageEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 监听 World 卸载事件
    FWorldDelegates::OnWorldCleanup.AddUObject(
        this, &UStageEditorSubsystem::OnWorldCleanup);
}

void UStageEditorSubsystem::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
    // 从缓存中移除该 World 的 RegistryAsset
    FSoftObjectPath WorldPath(World);

    TArray<FSoftObjectPath> ToRemove;
    for (const auto& Pair : LoadedRegistries)
    {
        if (Pair.Value->OwningLevel.ToSoftObjectPath() == WorldPath)
        {
            ToRemove.Add(Pair.Key);
        }
    }

    for (const FSoftObjectPath& Path : ToRemove)
    {
        LoadedRegistries.Remove(Path);
    }
}
```

**风险**: 🟠 中 - 多 World 编辑时可能出现 Registry 混乱

---

### 6. PostLoad 事件订阅时机问题

**Michael Nygard - 可靠性工程**:
> ⚠️ **竞态条件**: `EditorSubsystem::Initialize()` 时订阅 `OnStageLoaded` 事件，但 Stage Actor 可能在 Subsystem 初始化之前就被加载了。

**时序问题示例**:
```
启动编辑器 → 打开 Level:

时刻 1: UE 开始加载 Level 资产
时刻 2: Stage Actor 开始反序列化
时刻 3: Stage::PostLoad() 被调用
时刻 4: 尝试触发 OnStageLoaded 事件 ← Subsystem 还未初始化！
时刻 5: EditorSubsystem::Initialize() 被调用
时刻 6: 订阅 OnStageLoaded 事件 ← 太晚了，已经错过时刻 4
```

**建议补充**:
```cpp
// 1. 在 EditorSubsystem 初始化时扫描已加载的 Stage
void UStageEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 订阅未来的事件
    // （实际实现需要遍历所有 World 并订阅它们的 StageManagerSubsystem）

    // 处理已经加载的 Stage（补偿逻辑）
    ScanAndRegisterLoadedStages();
}

void UStageEditorSubsystem::ScanAndRegisterLoadedStages()
{
    // 遍历所有已加载的 World
    for (TObjectIterator<UWorld> It; It; ++It)
    {
        UWorld* World = *It;
        if (!World || World->IsTemplate())
            continue;

        // 查找该 World 的所有 Stage Actor
        for (TActorIterator<AStage> ActorIt(World); ActorIt; ++ActorIt)
        {
            AStage* Stage = *ActorIt;

            // 模拟 HandleStageLoaded 逻辑
            if (Stage->StageID == 0)
            {
                // 未注册的 Stage，执行注册
                UStageRegistryAsset* Registry = GetOrLoadRegistryAsset(World);
                if (Registry)
                {
                    RegisterStageInternal(Registry, Stage);
                }
            }
            else
            {
                // 已注册，添加到运行时缓存
                UStageManagerSubsystem* RuntimeSub =
                    World->GetSubsystem<UStageManagerSubsystem>();
                if (RuntimeSub)
                {
                    RuntimeSub->AddStageToRuntimeCache(Stage);
                }
            }
        }
    }
}

// 2. 在 Stage::PostLoad 中添加防御性检查
void AStage::PostLoad()
{
    Super::PostLoad();

#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World || World->IsPlayInEditor() || World->IsGameWorld())
        return;

    UStageManagerSubsystem* RuntimeSub = World->GetSubsystem<UStageManagerSubsystem>();
    if (!RuntimeSub)
    {
        // Subsystem 还未初始化，稍后会被 ScanAndRegisterLoadedStages() 处理
        UE_LOG(LogStageEditor, Verbose,
            TEXT("StageManagerSubsystem not ready, Stage %s will be registered later"),
            *GetName());
        return;
    }

    // 触发事件
    RuntimeSub->OnStageLoaded(this);
#endif
}
```

**风险**: 🟠 中 - 编辑器启动时加载的 Stage 可能不会被正确注册

---

### 7. 批量加载性能问题

**Michael Nygard - 性能优化**:
> ⚠️ **性能问题**: 如果 World Partition 批量加载 1000 个 Stage，会触发 1000 次 `HandleStageLoaded()`，每次都可能加载 RegistryAsset 和执行 Source Control 操作。

**性能瓶颈分析**:
```cpp
// 当前设计：每个 Stage 触发一次处理
Stage_001::PostLoad() → HandleStageLoaded() → GetOrLoadRegistryAsset() → Load from disk
Stage_002::PostLoad() → HandleStageLoaded() → GetOrLoadRegistryAsset() → Cache hit
Stage_003::PostLoad() → HandleStageLoaded() → GetOrLoadRegistryAsset() → Cache hit
...
Stage_1000::PostLoad() → HandleStageLoaded() → GetOrLoadRegistryAsset() → Cache hit

问题：
1. 1000 次函数调用开销
2. 如果是多人模式，第一个 Stage 触发 Check Out，阻塞后续操作
3. 大量日志输出影响性能
```

**建议优化**:
```cpp
// 1. 批量处理机制
class UStageEditorSubsystem : public UEditorSubsystem
{
private:
    // 待处理的 Stage 队列
    TArray<AStage*> PendingStages;

    // 定时器句柄
    FTimerHandle BatchProcessTimerHandle;

public:
    void HandleStageLoaded(AStage* Stage)
    {
        // 添加到队列，而不是立即处理
        PendingStages.Add(Stage);

        // 启动延迟批量处理（100ms 后执行）
        if (!BatchProcessTimerHandle.IsValid())
        {
            GetWorld()->GetTimerManager().SetTimer(
                BatchProcessTimerHandle,
                this, &UStageEditorSubsystem::ProcessPendingStagesBatch,
                0.1f, false);
        }
    }

    void ProcessPendingStagesBatch()
    {
        if (PendingStages.Num() == 0)
            return;

        UE_LOG(LogStageEditor, Log,
            TEXT("Processing %d Stages in batch"), PendingStages.Num());

        // 按 World 分组
        TMap<UWorld*, TArray<AStage*>> StagesByWorld;
        for (AStage* Stage : PendingStages)
        {
            UWorld* World = Stage->GetWorld();
            if (World)
            {
                StagesByWorld.FindOrAdd(World).Add(Stage);
            }
        }

        // 逐 World 处理
        for (const auto& Pair : StagesByWorld)
        {
            UWorld* World = Pair.Key;
            const TArray<AStage*>& Stages = Pair.Value;

            // 一次性加载 Registry（而不是每个 Stage 加载一次）
            UStageRegistryAsset* Registry = GetOrLoadRegistryAsset(World);
            if (!Registry)
                continue;

            // 多人模式：一次性 Check Out
            if (Registry->CollaborationMode == ECollaborationMode::Multi)
            {
                CheckOutRegistryFile(Registry);
            }

            // 批量处理
            UStageManagerSubsystem* RuntimeSub =
                World->GetSubsystem<UStageManagerSubsystem>();

            for (AStage* Stage : Stages)
            {
                if (Stage->StageID != 0)
                {
                    // 已注册
                    RuntimeSub->AddStageToRuntimeCache(Stage);
                }
                else
                {
                    // 未注册
                    RegisterStageInternal(Registry, Stage);
                }
            }

            // 一次性保存
            if (Registry->IsDirty())
            {
                SaveRegistryAsset(Registry);
            }
        }

        // 清空队列
        PendingStages.Empty();
        BatchProcessTimerHandle.Invalidate();

        UE_LOG(LogStageEditor, Log, TEXT("Batch processing completed"));
    }
};
```

**性能提升**:
- Registry 加载：1000 次 → 1 次
- Source Control Check Out：1000 次 → 1 次
- 磁盘保存：1000 次 → 1 次
- 预计加载时间从 ~10 秒降至 ~0.5 秒

**风险**: 🟡 中 - 大型项目加载时会有明显卡顿

---

## 🟡 P2 - 优化建议（提升用户体验）

### 8. 缺少数据完整性验证工具

**Karl Wiegers - 质量保障**:
> 💡 **建议**: 提供一个验证工具，让用户定期检查 Registry 和实际 Stage Actor 的一致性。

**建议添加**:
```cpp
// 在 StageEditor 菜单中添加验证工具
Menu: Tools → StageEditor → Validate Registry Integrity

// 验证结果对话框
┌─────────────────────────────────────────────────────────────┐
│  Registry 完整性验证报告                                     │
├─────────────────────────────────────────────────────────────┤
│  World: ForestLevel                                          │
│  Registry: /Game/StageEditor/Registries/ForestLevel_...     │
│                                                              │
│  ✅ 检查通过 (12 项)                                         │
│  ⚠️ 发现 2 个警告                                            │
│  ❌ 发现 1 个错误                                            │
│                                                              │
│  ❌ 错误: StageID=5 在 Registry 中，但 Actor 不存在          │
│     建议: 从 Registry 移除此条目                             │
│     [🗑️ 移除条目]                                           │
│                                                              │
│  ⚠️ 警告: Stage_NewArea 存在但未注册 (StageID=0)             │
│     建议: 注册此 Stage                                       │
│     [➕ 注册 Stage]                                          │
│                                                              │
│  ⚠️ 警告: StageID=3 的 ActCount 不匹配                       │
│     Registry: 5, Actual: 7                                   │
│     建议: 同步 Registry                                      │
│     [🔄 同步]                                                │
│                                                              │
│  [🔧 自动修复所有]  [📄 导出报告]  [✅ 关闭]                 │
└─────────────────────────────────────────────────────────────┘
```

**优先级**: P2 - 不阻塞实施，但强烈建议在 Phase 13.1 实现

---

### 9. StageID 重用机制

**Gojko Adzic - 实用性**:
> 💡 **优化**: 当前设计中 `NextStageID` 单调递增永不重用。长期运行后 ID 可能很大（例如 1000+），如果用户删除了很多 Stage，中间会有大量空洞。

**建议添加 ID 池机制**:
```cpp
UCLASS()
class UStageRegistryAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    /** 下一个可分配的 StageID */
    UPROPERTY(VisibleAnywhere, Category = "Registry")
    int32 NextStageID = 1;

    /** 已删除的 StageID 池（可重用） */
    UPROPERTY(VisibleAnywhere, Category = "Registry")
    TArray<int32> FreeStageIDs;

    /** Stage 条目列表 */
    UPROPERTY(VisibleAnywhere, Category = "Registry")
    TArray<FStageRegistryEntry> StageEntries;

    // 分配 ID：优先使用 FreeStageIDs，其次使用 NextStageID
    int32 AllocateNewID()
    {
        if (FreeStageIDs.Num() > 0)
        {
            // 重用已删除的 ID
            int32 ReusedID = FreeStageIDs.Pop();
            UE_LOG(LogStageEditor, Log,
                TEXT("Reusing freed StageID: %d"), ReusedID);
            return ReusedID;
        }
        else
        {
            // 分配新 ID
            return NextStageID++;
        }
    }

    // 移除条目：将 ID 添加到 FreeStageIDs
    bool RemoveEntry(int32 StageID)
    {
        int32 RemovedIndex = StageEntries.IndexOfByPredicate(
            [StageID](const FStageRegistryEntry& Entry) {
                return Entry.StageID == StageID;
            });

        if (RemovedIndex != INDEX_NONE)
        {
            StageEntries.RemoveAt(RemovedIndex);
            FreeStageIDs.Add(StageID);  // 加入重用池
            UE_LOG(LogStageEditor, Log,
                TEXT("StageID %d added to free pool"), StageID);
            return true;
        }
        return false;
    }
};
```

**收益**:
- ID 保持在较小范围内（例如始终 <100）
- 更容易记忆和调试
- 节省内存（虽然微不足道）

**成本**: +50 行代码

**优先级**: P2 - 可选优化，对功能无影响

---

### 10. 国际化支持

**Karl Wiegers - 完整性检查**:
> 💡 **提醒**: 项目已有本地化支持（Phase 7），新增的 UI 字符串需要整合到本地化系统。

**需要本地化的字符串**:
```cpp
// 1. 创建 Registry 对话框
LOCTEXT("CreateRegistryDialog_Title", "创建 Stage Registry")
LOCTEXT("CreateRegistryDialog_CollaborationMode", "协作模式:")
LOCTEXT("CreateRegistryDialog_SoloMode", "单人开发")
LOCTEXT("CreateRegistryDialog_MultiMode", "多人协作")

// 2. 警告条 UI
LOCTEXT("RegistryWarning_Title", "当前 Level 未初始化 Stage Registry")
LOCTEXT("RegistryWarning_Action", "请先创建 Registry 才能使用 StageEditor")

// 3. 协作模式提示
LOCTEXT("SoloModeWarning", "⚠️ 重要提示: 创建后无法切换为多人协作模式")
LOCTEXT("MultiModeWarning", "⚠️ 重要提示: 所有协作者都需要启用 Source Control")

// 4. 错误消息
LOCTEXT("Error_RegistryNotFound", "未找到 Registry，请先创建")
LOCTEXT("Error_CheckOutFailed", "无法 Check Out Registry 文件，请检查 Source Control 状态")
```

**建议**: 在实施时将所有 UI 字符串加入本地化表（`.ini` 文件）

---

## 📊 代码量修正估计

### 原文档估计: ~1350 行

### 修正后估计: ~2100 行 (+55%)

| 模块 | 原估计 | 修正后 | 差异原因 |
|------|--------|--------|---------|
| 新建文件（Runtime） | 300 | 350 | +FreeStageIDs 机制 |
| 新建文件（Editor） | 250 | 300 | +EditorSubsystem 生命周期处理 |
| Runtime 模块修改 | 200 | 250 | +PostLoad 防御性检查 |
| Editor 模块修改 | 600 | 900 | +错误处理 +并发冲突解决 +批量处理 |
| **错误处理和边界情况** | 0 | 200 | **新增** |
| **UI 实现（对话框+警告条）** | (含在 600 中) | 300 | **独立估算** |
| **迁移工具和验证** | 0 | 200 | **新增** |
| **国际化字符串** | 0 | 50 | **新增** |
| **测试代码（建议）** | 0 | 200 | **新增** |
| **总计** | **1350** | **~2100** | **+750 (+55%)** |

**建议**: 在实施计划中预留 2-3 周时间（而非原估计的 1-2 周）

---

## ✅ 总结与建议

### 可实施性结论

**当前状态**: ✅ **基本可实施**，但需要补充关键细节才能开始编码。

**达到生产就绪的待办事项**:

#### 立即处理（P0）- 估计 2-3 天

1. ✅ **验证 FLevelInstanceID 稳定性**
   - 查阅 UE 源码确认 Hash 计算机制
   - 编写测试代码验证多次加载的一致性
   - 补充判空逻辑说明

2. ✅ **补充事务一致性处理**
   - 在 `RegisterStage()` 中添加 Check Out 预检查
   - 添加保存失败回滚逻辑
   - 在 `CreateNewStage()` 中处理注册失败

3. ✅ **详细化迁移方案**
   - 设计迁移对话框 UI
   - 实现 `ValidateRegistryIntegrity()` 验证工具
   - 编写迁移前的数据备份逻辑

#### 短期处理（P1）- 估计 3-5 天

4. ✅ **补充 Git 并发冲突解决**
   - 实现 `DetectRegistryConflicts()` 检测工具
   - 设计冲突解决对话框
   - 实现 `AutoResolveConflicts()` 自动解决逻辑

5. ✅ **修复 EditorSubsystem 生命周期问题**
   - 实现 `ScanAndRegisterLoadedStages()` 补偿逻辑
   - 添加 World 卸载事件监听
   - 在 `Stage::PostLoad` 中添加防御性检查

6. ✅ **实现批量加载优化**
   - 添加 `PendingStages` 队列和定时器
   - 实现 `ProcessPendingStagesBatch()` 批量处理
   - 性能测试验证改进效果

#### 中期优化（P2）- 估计 2-3 天

7. ✅ **开发数据完整性验证工具**
   - 实现菜单命令和验证逻辑
   - 设计验证报告对话框
   - 添加自动修复功能

8. ✅ **添加 StageID 重用机制**（可选）
   - 在 `UStageRegistryAsset` 中添加 `FreeStageIDs` 数组
   - 修改 `AllocateNewID()` 和 `RemoveEntry()` 逻辑

9. ✅ **整合国际化支持**
   - 提取所有 UI 字符串到 `LOCTEXT` 宏
   - 添加到本地化表

---

### 最终评分

| 评审维度 | 原始评分 | 补充后预期评分 |
|---------|---------|--------------|
| 核心架构 | 9/10 | 9/10 |
| 技术细节 | 6/10 | **9/10** ⬆️ |
| 错误处理 | 5/10 | **8/10** ⬆️ |
| 实施计划 | 4/10 | **8/10** ⬆️ (如果补充计划) |
| 测试策略 | 3/10 | **7/10** ⬆️ (如果添加测试) |
| **总体可实施性** | **7.5/10** | **9/10** ⬆️ |

---

### 推荐实施路径

**Phase 13.1 - 核心实现 (Week 1-2)**
1. 实现 RegistryAsset 和数据结构
2. 实现 EditorSubsystem 基础功能（含 P0 修复）
3. 重构 StageManagerSubsystem 为纯运行时
4. 实现迁移逻辑和验证工具

**Phase 13.2 - 质量增强 (Week 3)**
5. 实现 Git 冲突解决机制
6. 实现批量加载优化
7. 添加完整性验证工具
8. 编写单元测试和集成测试

**Phase 13.3 - 打磨和优化 (Week 4)**
9. UI 完善和本地化
10. 性能测试和优化
11. 文档更新
12. 用户手册编写

---

## 专家签名

**Martin Fowler** - 架构设计
*"架构设计清晰合理，建议补充多 World 场景处理和生命周期管理。"*

**Michael Nygard** - 可靠性工程
*"需要加强错误处理和并发控制，特别是 Git 场景下的冲突解决。"*

**Karl Wiegers** - 需求工程
*"核心需求明确，但缺少验收标准和测试策略，建议补充迁移方案细节。"*

**Gojko Adzic** - 可测试性
*"需要补充可测试的验收场景（Given/When/Then），以及数据完整性验证机制。"*

---

*评审完成日期: 2025-12-04*
*下一步: 根据 P0 和 P1 建议更新 Phase13_StageRegistry_Discussion.md，补充第 8 节实施计划*
