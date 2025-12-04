# Phase 3: 命名解析模块

> 日期: 2025-11-29 13:00-13:05
> 状态: ✅ 完成

---

## 1. 目标

实现 DataLayer 名称解析器，识别符合 StageEditor 命名规范的 DataLayer：
- Stage 级别: `DL_Stage_<StageName>`
- Act 级别: `DL_Act_<StageName>_<ActName>`

---

## 2. 实施记录

### 2.1 创建的文件

| 文件 | 说明 |
|------|------|
| `Public/DataLayerSync/StageDataLayerNameParser.h` | 解析器头文件 |
| `Private/DataLayerSync/StageDataLayerNameParser.cpp` | 解析器实现 |

### 2.2 解析结果结构体

**StageDataLayerNameParser.h:18-49**

```cpp
USTRUCT(BlueprintType)
struct STAGEEDITOR_API FDataLayerNameParseResult
{
    GENERATED_BODY()

    bool bIsValid = false;       // 是否符合命名规范
    bool bIsStageLayer = false;  // Stage(true) 或 Act(false)
    FString StageName;           // 对两种级别都有效
    FString ActName;             // 仅 Act 级别有效

    // 辅助函数
    FString ToDataLayerName() const;  // 重建 DataLayer 名称
    FString GetDisplayName() const;   // 获取友好显示名
};
```

### 2.3 正则解析逻辑

**StageDataLayerNameParser.cpp:14-42**

```cpp
FDataLayerNameParseResult FStageDataLayerNameParser::Parse(const FString& DataLayerName)
{
    FDataLayerNameParseResult Result;

    // Stage Pattern: ^DL_Stage_(.+)$
    static const FRegexPattern StagePattern(TEXT("^DL_Stage_(.+)$"));
    FRegexMatcher StageMatcher(StagePattern, DataLayerName);

    if (StageMatcher.FindNext())
    {
        Result.bIsValid = true;
        Result.bIsStageLayer = true;
        Result.StageName = StageMatcher.GetCaptureGroup(1);
        return Result;
    }

    // Act Pattern: ^DL_Act_([^_]+)_(.+)$
    // StageName 不能含下划线，ActName 可以
    static const FRegexPattern ActPattern(TEXT("^DL_Act_([^_]+)_(.+)$"));
    FRegexMatcher ActMatcher(ActPattern, DataLayerName);

    if (ActMatcher.FindNext())
    {
        Result.bIsValid = true;
        Result.bIsStageLayer = false;
        Result.StageName = ActMatcher.GetCaptureGroup(1);
        Result.ActName = ActMatcher.GetCaptureGroup(2);
        return Result;
    }

    return Result; // bIsValid = false
}
```

### 2.4 辅助函数

| 函数 | 用途 |
|------|------|
| `IsStageDataLayer()` | 快速检查是否 Stage 级别 |
| `IsActDataLayer()` | 快速检查是否 Act 级别 |
| `IsValidStageEditorDataLayer()` | 检查是否符合任一规范 |
| `MakeStageDataLayerName()` | 生成 `DL_Stage_<name>` |
| `MakeActDataLayerName()` | 生成 `DL_Act_<stage>_<act>` |

### 2.5 解析示例

| 输入 | bIsValid | bIsStageLayer | StageName | ActName |
|------|----------|---------------|-----------|---------|
| `DL_Stage_Forest` | true | true | Forest | - |
| `DL_Act_Forest_Day` | true | false | Forest | Day |
| `DL_Act_Forest_Night_Rain` | true | false | Forest | Night_Rain |
| `MyDataLayer` | false | - | - | - |

### 2.6 编译验证

```
Result: Succeeded
Total execution time: 13.33 seconds
```

---

*文档结束*
