# World Partition 转换后的 MapCheck 错误解析

## 错误信息

```
MapCheck: Error: World references spatially loaded actor /Game/Maps/ToolTestMap1222.BP_Cube_2
```

---

## 问题解析

### 这个错误是什么意思？

**核心原因：**
在 World Partition 中，Actors 被分块存储（spatially loaded），但是 **Persistent Level（常驻关卡）仍然持有对这些 Actors 的硬引用**。

**技术细节：**

#### 转换前（传统 Level）
```
Persistent Level
├── BP_Cube_2 ✅ (直接存储)
├── BP_Cube_4 ✅ (直接存储)
├── SM_Chair ✅ (直接存储)
└── ...

引用方式: 硬引用（Hard Reference）OK
```

#### 转换后（World Partition）
```
Persistent Level (常驻)
├── WorldSettings
├── WorldDataLayers
└── (空的，不应该直接存储 Actors)

World Partition Grid Cells (空间分块)
├── Cell_0_0
│   ├── BP_Cube_2 ⚠️ (spatially loaded)
│   └── SM_Chair
├── Cell_0_1
│   └── BP_Cube_4 ⚠️ (spatially loaded)
└── ...

引用方式: ❌ Persistent Level 仍然硬引用这些 Actors → 错误！
```

**World Partition 规则：**
- ✅ Persistent Level 应该只包含 WorldSettings、WorldDataLayers 等元数据
- ❌ Persistent Level **不应该**直接引用 spatially loaded actors
- ✅ 应该使用 **Soft Object References** 或运行时动态查找

---

## 常见原因

### 1. Level Blueprint 引用 ⭐ (最常见)

**场景：**
你在 Level Blueprint 中直接引用了场景中的 Actors。

**示例：**
```blueprint
// Level Blueprint
Event BeginPlay
    ├─ Get Actor Reference → BP_Cube_2 ❌ (硬引用)
    └─ Set Material
```

**检查方式：**
1. 打开 Level Blueprint (`Blueprints → Open Level Blueprint`)
2. 查找所有 Actor 引用节点
3. 看是否有 `BP_Cube_2`、`SM_Chair` 等

**修复：**
- 方案 A：删除这些引用（如果不需要）
- 方案 B：改用 Soft Object Reference
- 方案 C：使用 Tag 或 Name 动态查找

---

### 2. Blueprint Actor 之间的引用

**场景：**
某个 Blueprint Actor 持有对其他 Actors 的引用。

**示例：**
```cpp
// BP_SomeActor.h
UPROPERTY(EditAnywhere)
AActor* TargetActor; // ❌ 如果指向 BP_Cube_2，会产生硬引用
```

**检查方式：**
1. 打开每个 Blueprint
2. 查看 Details 面板中的引用属性
3. 看是否指向了场景中的其他 Actors

**修复：**
```cpp
// 改用 Soft Object Reference
UPROPERTY(EditAnywhere)
TSoftObjectPtr<AActor> TargetActor; // ✅ 软引用
```

---

### 3. Component 引用

**场景：**
某个组件引用了场景中的其他 Actors。

**示例：**
```cpp
// UMyComponent
UPROPERTY(EditAnywhere)
AActor* LinkedActor; // ❌ 硬引用
```

**修复：**
同样改用 `TSoftObjectPtr<AActor>` 或运行时查找。

---

### 4. Data Assets 或配置文件引用

**场景：**
某个 Data Asset 或配置中引用了关卡中的 Actors。

**检查：**
搜索项目中所有引用这些 Actors 的资产。

---

## 快速修复方案

### 方案 1：清理 Level Blueprint ⭐ (推荐优先尝试)

**步骤：**

1. **打开 Level Blueprint**
   ```
   Blueprints → Open Level Blueprint
   ```

2. **查找所有 Actor 引用**
   - 右键 → `Find References`
   - 搜索 `BP_Cube`、`SM_Chair` 等

3. **删除或修改引用**

   **选项 A：删除不需要的逻辑**
   ```blueprint
   // 如果这些逻辑不重要，直接删除
   Event BeginPlay → Get BP_Cube_2 → ... (删除整个节点链)
   ```

   **选项 B：改用 Tag 查找**
   ```blueprint
   Event BeginPlay
       ├─ Get Actors with Tag → "MyCube"
       └─ For Each Loop
           └─ Set Material
   ```

4. **保存并重新编译**

---

### 方案 2：使用 Soft References

**如果必须保留引用：**

#### Blueprint 中：
```blueprint
// 变量类型改为 Soft Object Reference
UPROPERTY(EditAnywhere, Category = "References")
TSoftObjectPtr<AActor> TargetActor;

// 使用时加载
Event BeginPlay
    ├─ Load Asset (Async)
    │   Input: TargetActor
    └─ On Loaded
        └─ Use Actor
```

#### C++ 中：
```cpp
// Header
UPROPERTY(EditAnywhere, Category = "References")
TSoftObjectPtr<AActor> TargetActor;

// Usage
void AMyActor::BeginPlay()
{
    Super::BeginPlay();

    if (TargetActor.IsValid())
    {
        AActor* LoadedActor = TargetActor.Get(); // 获取 Actor
        if (LoadedActor)
        {
            // 使用 Actor
        }
    }
}
```

---

### 方案 3：运行时动态查找

**使用 Actor Tag 或 Name：**

#### 设置 Tag：
```
1. 选中 BP_Cube_2
2. Details → Tags → Add Tag → "MyCube"
```

#### 查找代码：
```cpp
// Blueprint
TArray<AActor*> FoundActors;
UGameplayStatics::GetAllActorsWithTag(GetWorld(), "MyCube", FoundActors);

// C++
TArray<AActor*> FoundActors;
UGameplayStatics::GetAllActorsWithTag(World, FName("MyCube"), FoundActors);
if (FoundActors.Num() > 0)
{
    AActor* TargetActor = FoundActors[0];
    // 使用 Actor
}
```

---

### 方案 4：使用 Find Actor by Name

```cpp
// Blueprint
AActor* FoundActor = UGameplayStatics::GetActorOfClass(
    GetWorld(),
    ABP_Cube_C::StaticClass()
);

// C++
for (TActorIterator<AActor> It(World); It; ++It)
{
    if (It->GetName() == "BP_Cube_2")
    {
        AActor* TargetActor = *It;
        // 使用 Actor
        break;
    }
}
```

---

## 针对 StageEditor 的特殊处理

### StageEditor 已经使用了正确的引用方式 ✅

**好消息：** StageEditor 的设计已经符合 World Partition 最佳实践！

```cpp
// AStage.h:43 - 使用 TSoftObjectPtr
TMap<int32, TSoftObjectPtr<AActor>> PropRegistry;

// UStagePropComponent.h - 使用 TSoftObjectPtr
TSoftObjectPtr<AStage> OwnerStage;
```

**这意味着：**
- ✅ StageEditor 不会产生这类错误
- ✅ Stage 和 Prop 之间的引用是软引用
- ✅ 完全兼容 World Partition

**但是：**
- ⚠️ 如果你在 **Level Blueprint** 中引用了 Stage 或 Prop Actors，仍然会有问题
- ⚠️ 如果你的 **自定义 Blueprint** 中硬引用了 Actors，需要修复

---

## 验证和修复步骤

### Step 1: 检查 Level Blueprint

```
1. Blueprints → Open Level Blueprint
2. 查看是否有红色的 Actor 引用节点
3. 删除或改用动态查找
4. Compile + Save
```

### Step 2: 检查所有 Blueprint Actors

```
1. 在 Content Browser 中搜索 "BP_"
2. 逐个打开 Blueprint
3. 检查 Details 面板中的引用属性
4. 改用 TSoftObjectPtr 或删除引用
```

### Step 3: 运行 Map Check

```
1. Tools → Map Check
2. 查看是否还有错误
3. 如果有，双击错误跳转到问题位置
```

### Step 4: 重新保存关卡

```
1. File → Save Current Level
2. 关闭并重新打开关卡
3. 再次运行 Map Check
```

---

## 快捷修复脚本（高级）

如果你有大量 Actors，可以使用 Python 脚本批量修复：

```python
import unreal

# 获取所有 Level Blueprint 中的引用
level_blueprint = unreal.EditorLevelLibrary.get_level_script_blueprint(
    unreal.EditorLevelLibrary.get_editor_world()
)

# 查找所有硬引用
# (需要手动检查和修复)
print(f"Level Blueprint: {level_blueprint.get_name()}")
```

---

## 这些错误是否影响使用？

### 短期影响

**编辑器中：**
- ⚠️ Map Check 会显示警告/错误
- ⚠️ 可能影响关卡的自动保存
- ⚠️ 某些工具可能拒绝操作

**运行时：**
- ✅ 游戏仍然可以运行
- ⚠️ 但是引用的 Actors 可能无法正确加载
- ⚠️ 可能导致空引用错误

### 长期影响

**如果不修复：**
- ❌ 无法发布最终版本（Package 会失败）
- ❌ 多人协作时会有问题
- ❌ 性能优化受限

**建议：**
- ✅ 尽快修复这些错误
- ✅ 使用软引用替代硬引用
- ✅ 遵循 World Partition 最佳实践

---

## 最佳实践总结

### ✅ 推荐做法

1. **使用 Soft Object References**
   ```cpp
   TSoftObjectPtr<AActor> MyActor;
   ```

2. **使用 Actor Tags 动态查找**
   ```cpp
   UGameplayStatics::GetAllActorsWithTag(World, "MyTag", OutActors);
   ```

3. **使用 Gameplay Tags**
   ```cpp
   FGameplayTag ActorTag = FGameplayTag::RequestGameplayTag("Actor.Type.Cube");
   ```

4. **避免 Level Blueprint 中的 Actor 引用**
   - 改用 Event Dispatcher
   - 改用 Tag 查找
   - 改用 Interface 通信

### ❌ 避免做法

1. **Level Blueprint 中硬引用 Actors**
   ```blueprint
   Get BP_Cube_2 ❌ // 不要这样做
   ```

2. **Blueprint 中使用硬引用**
   ```cpp
   UPROPERTY()
   AActor* MyActor; ❌ // 应该用 TSoftObjectPtr
   ```

3. **在 Construction Script 中引用其他 Actors**
   ```blueprint
   Construction Script → Get Other Actor ❌
   ```

---

## 总结

### 你的情况

**问题：**
- 转换后出现 30+ 个 MapCheck 错误
- 都是 "World references spatially loaded actor"

**原因：**
- 最可能是 **Level Blueprint** 中有引用
- 或者某些 Blueprint Actors 之间有硬引用

**修复优先级：**
1. ⭐ 检查并清理 Level Blueprint（最可能）
2. ⭐ 检查 Blueprint Actors 之间的引用
3. 运行 Map Check 验证
4. 重新保存关卡

**StageEditor：**
- ✅ StageEditor 本身没有问题（已使用软引用）
- ✅ 这些错误与 StageEditor 无关
- ✅ 是 World Partition 转换的正常现象

---

## 下一步行动

1. **立即检查 Level Blueprint**
   ```
   Blueprints → Open Level Blueprint
   查找所有 Actor 引用
   ```

2. **如果没有重要逻辑**
   ```
   直接删除所有引用
   Compile + Save
   ```

3. **重新运行 Map Check**
   ```
   Tools → Map Check
   ```

4. **如果错误消失**
   ```
   ✅ 问题解决！
   ```

5. **如果错误仍在**
   ```
   检查其他 Blueprint Actors
   搜索引用这些 Actors 的资产
   ```
