# SUID 唯一性设计讨论

> 状态：待讨论
> 创建日期：2025-11-27
> 相关代码：`StageCoreTypes.h` - `FSUID`

---

## 问题描述

当前 SUID 结构为 `StageID.ActID.PropID`，存在唯一性冲突：

| 元素类型 | SUID 值 | 冲突 |
|----------|---------|------|
| Stage 1 | `1.0.0` | ⚠️ |
| Default Act (ActID=0) | `1.0.0` | ⚠️ 与 Stage 重复 |
| Act 1 | `1.1.0` | ✅ |
| Prop 5 | `1.0.5` | ✅ |

---

## 当前 FSUID 结构

```cpp
struct FSUID
{
    int32 StageID = 0;  // Stage 的全局唯一 ID
    int32 ActID = 0;    // Act 在 Stage 内的 ID (0 = Default Act)
    int32 PropID = 0;   // Prop 在 Stage 内的 ID

    FString ToString() const
    {
        return FString::Printf(TEXT("%d.%d.%d"), StageID, ActID, PropID);
    }
};
```

---

## 方案对比

### 方案 A：添加类型字段

在 SUID 中增加 `Type` 字段来区分元素类型。

```cpp
enum class ESUIDType : uint8
{
    Stage = 0,
    Act = 1,
    Prop = 2
};

struct FSUID
{
    ESUIDType Type = ESUIDType::Stage;
    int32 StageID = 0;
    int32 ActID = 0;
    int32 PropID = 0;

    FString ToString() const
    {
        const TCHAR* Prefix = Type == ESUIDType::Stage ? TEXT("S") :
                              Type == ESUIDType::Act ? TEXT("A") : TEXT("P");
        return FString::Printf(TEXT("%s_%d.%d.%d"), Prefix, StageID, ActID, PropID);
    }
};
```

**示例：**
- Stage 1: `S_1.0.0`
- Default Act: `A_1.0.0`
- Act 1: `A_1.1.0`
- Prop 5: `P_1.0.5`

**优点：**
- 完全消除歧义
- 字符串表示直观
- 向后兼容（可以解析旧格式）

**缺点：**
- 结构体增加一个字段
- 需要更新所有序列化/反序列化代码

---

### 方案 B：变长格式（按类型省略无用字段）

不同类型使用不同长度的 SUID：

```cpp
struct FSUID
{
    int32 StageID = 0;
    int32 ActID = -1;   // -1 表示不适用
    int32 PropID = -1;  // -1 表示不适用

    FString ToString() const
    {
        if (PropID >= 0)
            return FString::Printf(TEXT("P:%d.%d"), StageID, PropID);
        else if (ActID >= 0)
            return FString::Printf(TEXT("A:%d.%d"), StageID, ActID);
        else
            return FString::Printf(TEXT("S:%d"), StageID);
    }
};
```

**示例：**
- Stage 1: `S:1`
- Default Act: `A:1.0`
- Act 1: `A:1.1`
- Prop 5: `P:1.5`

**优点：**
- 更简洁的表示
- 明确反映层级关系

**缺点：**
- 解析复杂度增加
- 需要处理 -1 的边界情况

---

### 方案 C：Default Act 使用特殊 ID

保持当前结构，但 Default Act 使用特殊值（如 -1 或 MAX_INT）：

```cpp
// Default Act ID 改为 -1 或使用常量
static constexpr int32 DEFAULT_ACT_ID = -1;

// Stage: 1.0.0
// Default Act: 1.-1.0
// Act 1: 1.1.0
```

**示例：**
- Stage 1: `1.0.0`
- Default Act: `1.-1.0` 或 `1.*.0`
- Act 1: `1.1.0`
- Prop 5: `1.0.5`

**优点：**
- 最小改动
- 数字格式统一

**缺点：**
- -1 作为 ID 不够直观
- 可能影响 ActID >= 0 的假设

---

### 方案 D：重新定义 SUID 语义

SUID 仅用于**可寻址元素**，Stage 本身不需要 SUID（它是容器）：

```cpp
// Stage 不参与 SUID 系统，只有 Acts 和 Props 有 SUID
// Act SUID: StageID.ActID
// Prop SUID: StageID.PropID

struct FSUID
{
    int32 StageID = 0;
    int32 LocalID = 0;  // ActID 或 PropID
    bool bIsAct = true; // 类型标记
};
```

**优点：**
- 结构更简单
- Stage 作为容器，概念上不需要 ID

**缺点：**
- Stage 确实需要一个标识符用于跨 Stage 引用
- 改变了现有设计理念

---

## 推荐方案

**推荐：方案 A（添加类型字段）**

理由：
1. **清晰性**：类型字段明确表达元素类型，消除所有歧义
2. **扩展性**：未来可以轻松添加新类型（如 Event、Trigger 等）
3. **调试友好**：字符串格式 `S_1.0.0` 一眼可知是 Stage
4. **改动可控**：只需修改 FSUID 结构，不改变业务逻辑

---

## 实施计划（如果采用方案 A）

### 阶段 1：更新 FSUID 结构
```cpp
// StageCoreTypes.h
enum class ESUIDType : uint8
{
    Invalid = 0,
    Stage = 1,
    Act = 2,
    Prop = 3
};

USTRUCT(BlueprintType)
struct FSUID
{
    GENERATED_BODY()

    UPROPERTY()
    ESUIDType Type = ESUIDType::Invalid;

    UPROPERTY()
    int32 StageID = 0;

    UPROPERTY()
    int32 ActID = 0;

    UPROPERTY()
    int32 PropID = 0;

    // Factory methods
    static FSUID MakeStage(int32 InStageID);
    static FSUID MakeAct(int32 InStageID, int32 InActID);
    static FSUID MakeProp(int32 InStageID, int32 InPropID);

    FString ToString() const;
    bool IsValid() const { return Type != ESUIDType::Invalid && StageID > 0; }
};
```

### 阶段 2：更新使用点
- `AStage::SUID` - 设置 Type = Stage
- `FAct::SUID` - 设置 Type = Act
- `UStagePropComponent::SUID` - 设置 Type = Prop

### 阶段 3：更新 UI 显示
- 已完成：`BuildDisplayText()` 已使用 `S_`、`A_`、`P_` 前缀

---

## 决策记录

| 日期 | 决策 | 原因 |
|------|------|------|
| 2025-11-27 | ✅ 采用简化方案：ActID 从 1 开始 | 最简单的解决方案，无需添加类型字段 |

### 最终方案：ActID 从 1 开始计数

**修改内容：**
- Default Act 的 ActID 从 0 改为 1
- 其他 Act 从 2 开始递增

**SUID 唯一性保证：**
| 元素类型 | SUID 值 | 唯一性 |
|----------|---------|--------|
| Stage 1 | `1.0.0` | ✅ |
| Default Act | `1.1.0` | ✅ |
| Act 2 | `1.2.0` | ✅ |
| Prop 1 | `1.0.1` | ✅ |

**修改的文件：**
- `Stage.cpp` - Default Act 初始化和查找逻辑
- `StageCoreTypes.h` - 注释更新

---

## 相关文档

- `Docs/StageEditor/HighLevelDesign.md` - SUID 系统设计
- `StageCoreTypes.h` - FSUID 定义
