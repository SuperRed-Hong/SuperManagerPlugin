# M-003 Stage Debug HUD - 实现交接文档

> 创建日期: 2025-11-27
> 状态: 待实现
> 优先级: 中

---

## 1. 项目背景

### 1.1 项目结构

```
ExtendEditor/
├── Plugins/
│   └── StageEditor/
│       ├── Source/
│       │   ├── StageEditorRuntime/     ← Runtime 模块（Debug HUD 放这里）
│       │   │   ├── Public/
│       │   │   │   ├── Actors/Stage.h
│       │   │   │   ├── Components/StageTriggerZoneComponent.h
│       │   │   │   ├── Core/StageCoreTypes.h
│       │   │   │   └── Subsystems/StageManagerSubsystem.h
│       │   │   └── Private/
│       │   │       └── ...
│       │   └── StageEditor/            ← Editor 模块
│       └── StageEditor.uplugin
└── Docs/StageEditor/
    └── ImplementationPlan/FutureTodoList.md
```

### 1.2 相关类说明

| 类名 | 路径 | 说明 |
|------|------|------|
| `AStage` | Actors/Stage.h | 舞台 Actor，管理 Acts 和 Props |
| `UStageManagerSubsystem` | Subsystems/StageManagerSubsystem.h | World Subsystem，管理所有 Stage 注册 |
| `EStageRuntimeState` | Core/StageCoreTypes.h | Stage 运行时状态枚举 |
| `UStageTriggerZoneComponent` | Components/StageTriggerZoneComponent.h | 触发区域组件 |

### 1.3 关键 API

```cpp
// 获取所有注册的 Stage
UStageManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UStageManagerSubsystem>();
TArray<AStage*> AllStages = Subsystem->GetAllStages();

// Stage 状态查询
EStageRuntimeState State = Stage->GetCurrentStageState();
bool bLocked = Stage->IsStageStateLocked();
TArray<int32> ActiveActs = Stage->GetActiveActIDs();
int32 LoadZoneActorCount = Stage->OverlappingLoadZoneActors.Num();
int32 ActivateZoneActorCount = Stage->OverlappingActivateZoneActors.Num();

// DataLayer 状态
EDataLayerRuntimeState DLState = Stage->GetStageDataLayerState();
```

---

## 2. 需求描述

### 2.1 核心需求

运行时 Debug 工具，在屏幕角落实时显示所有 Stage 和 DataLayer 的状态。

### 2.2 功能列表

| 功能 | 优先级 | 状态 |
|------|--------|------|
| 屏幕角落显示 Stage 状态列表 | 高 | 待实现 |
| 显示位置可配置（预设 + 自定义坐标） | 高 | 待实现 |
| 简洁/详细模式切换 | 高 | 待实现 |
| 控制台命令 `Stage.Debug 0/1` | 高 | 待实现 |
| Project Settings 配置面板 | 高 | 待实现 |
| 状态变化高亮效果 | 低 | 未计划 |
| 快捷键开关 | 低 | 未计划 |
| 过滤功能 | 低 | 未计划 |
| 编辑器模式支持 | 低 | 未计划 |

### 2.3 显示内容

```
【简洁模式】
Stage_1: Active | DL: Activated

【详细模式】
Stage_1 (ID:1)
├─ State: Active (Locked)
├─ StageDataLayer: Activated
├─ Acts: [0✓, 1✓, 2]  (✓=激活)
├─ LoadZone: 2 actors
└─ ActivateZone: 1 actor
```

---

## 3. 技术方案

### 3.1 架构图

```
Project Settings → Plugins → Stage Editor
         │
         ▼
┌─────────────────────────────┐
│   UStageDebugSettings       │  ← UDeveloperSettings
│   ├─ bEnableDebugHUD        │
│   ├─ DisplayPosition        │  ← 枚举: TopLeft/TopRight/BottomLeft/BottomRight/Custom
│   ├─ CustomOffset           │  ← FVector2D
│   └─ bDetailedMode          │
└─────────────────────────────┘
         │
         ▼ (读取配置)
┌─────────────────────────────┐
│   AStageDebugHUD : AHUD     │
│   └─ DrawHUD() override     │
│       ├─ 获取 Settings      │
│       ├─ 获取所有 Stage     │  ← StageManagerSubsystem
│       └─ Canvas 绘制        │
└─────────────────────────────┘
         │
         ▼ (控制)
┌─────────────────────────────┐
│   控制台命令                 │
│   └─ Stage.Debug 0/1        │
└─────────────────────────────┘
```

### 3.2 设计决策

| 问题 | 决策 | 理由 |
|------|------|------|
| 配置存储 | UDeveloperSettings | 统一管理，出现在 Project Settings |
| HUD 绘制 | AHUD::DrawHUD Canvas | 灵活控制布局，原生支持 |
| 模块归属 | StageEditorRuntime | 运行时需要，不依赖编辑器 |
| 编辑器模式 | 暂不支持 | 先满足运行时需求 |

---

## 4. 实现细节

### 4.1 需要创建的文件

```
Plugins/StageEditor/Source/StageEditorRuntime/
├── Public/
│   ├── Debug/
│   │   ├── StageDebugSettings.h      ← NEW
│   │   └── StageDebugHUD.h           ← NEW
├── Private/
│   ├── Debug/
│   │   ├── StageDebugSettings.cpp    ← NEW
│   │   └── StageDebugHUD.cpp         ← NEW
```

### 4.2 StageDebugSettings.h 骨架

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "StageDebugSettings.generated.h"

/**
 * 显示位置枚举
 */
UENUM(BlueprintType)
enum class EStageDebugPosition : uint8
{
    TopLeft      UMETA(DisplayName = "Top Left"),
    TopRight     UMETA(DisplayName = "Top Right"),
    BottomLeft   UMETA(DisplayName = "Bottom Left"),
    BottomRight  UMETA(DisplayName = "Bottom Right"),
    Custom       UMETA(DisplayName = "Custom Position")
};

/**
 * Stage Debug HUD 配置
 * 出现在 Project Settings → Plugins → Stage Editor
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Stage Editor"))
class STAGEEDITORRUNTIME_API UStageDebugSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UStageDebugSettings();

    /** 获取单例实例 */
    static UStageDebugSettings* Get();

    //----------------------------------------------------------------
    // Debug HUD Settings
    //----------------------------------------------------------------

    /** 是否启用 Debug HUD */
    UPROPERTY(config, EditAnywhere, Category = "Debug HUD")
    bool bEnableDebugHUD = false;

    /** 显示位置 */
    UPROPERTY(config, EditAnywhere, Category = "Debug HUD")
    EStageDebugPosition DisplayPosition = EStageDebugPosition::TopLeft;

    /** 自定义偏移（当 DisplayPosition = Custom 时生效） */
    UPROPERTY(config, EditAnywhere, Category = "Debug HUD",
        meta = (EditCondition = "DisplayPosition == EStageDebugPosition::Custom"))
    FVector2D CustomOffset = FVector2D(50.0f, 50.0f);

    /** 是否使用详细模式 */
    UPROPERTY(config, EditAnywhere, Category = "Debug HUD")
    bool bDetailedMode = true;

    /** 文字大小 */
    UPROPERTY(config, EditAnywhere, Category = "Debug HUD", meta = (ClampMin = "0.5", ClampMax = "3.0"))
    float TextScale = 1.0f;

    //----------------------------------------------------------------
    // UDeveloperSettings Interface
    //----------------------------------------------------------------

    virtual FName GetCategoryName() const override { return FName("Plugins"); }
    virtual FName GetSectionName() const override { return FName("Stage Editor"); }
};
```

### 4.3 StageDebugHUD.h 骨架

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "StageDebugHUD.generated.h"

class AStage;
enum class EStageRuntimeState : uint8;
enum class EDataLayerRuntimeState : uint8;

/**
 * Stage Debug HUD
 * 在屏幕上绘制所有 Stage 的状态信息
 */
UCLASS()
class STAGEEDITORRUNTIME_API AStageDebugHUD : public AHUD
{
    GENERATED_BODY()

public:
    AStageDebugHUD();

    virtual void DrawHUD() override;

protected:
    /** 绘制单个 Stage 的信息（简洁模式） */
    void DrawStageSimple(AStage* Stage, float& YOffset);

    /** 绘制单个 Stage 的信息（详细模式） */
    void DrawStageDetailed(AStage* Stage, float& YOffset);

    /** 获取状态对应的颜色 */
    FLinearColor GetStateColor(EStageRuntimeState State) const;

    /** 获取 DataLayer 状态对应的颜色 */
    FLinearColor GetDataLayerStateColor(EDataLayerRuntimeState State) const;

    /** 状态枚举转字符串 */
    static FString StateToString(EStageRuntimeState State);
    static FString DataLayerStateToString(EDataLayerRuntimeState State);

    /** 计算起始绘制位置 */
    FVector2D GetStartPosition() const;

    /** 行高 */
    float LineHeight = 18.0f;

    /** 缩进宽度 */
    float IndentWidth = 20.0f;
};
```

### 4.4 StageDebugHUD.cpp 关键实现

```cpp
#include "Debug/StageDebugHUD.h"
#include "Debug/StageDebugSettings.h"
#include "Subsystems/StageManagerSubsystem.h"
#include "Actors/Stage.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

AStageDebugHUD::AStageDebugHUD()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AStageDebugHUD::DrawHUD()
{
    Super::DrawHUD();

    // 检查是否启用
    UStageDebugSettings* Settings = UStageDebugSettings::Get();
    if (!Settings || !Settings->bEnableDebugHUD)
    {
        return;
    }

    // 获取 Subsystem
    UWorld* World = GetWorld();
    if (!World) return;

    UStageManagerSubsystem* Subsystem = World->GetSubsystem<UStageManagerSubsystem>();
    if (!Subsystem) return;

    // 获取所有 Stage
    TArray<AStage*> AllStages = Subsystem->GetAllStages();
    if (AllStages.Num() == 0) return;

    // 计算起始位置
    FVector2D StartPos = GetStartPosition();
    float YOffset = StartPos.Y;

    // 绘制标题
    const float Scale = Settings->TextScale;
    DrawText(TEXT("=== Stage Debug ==="), FLinearColor::Yellow, StartPos.X, YOffset, nullptr, Scale);
    YOffset += LineHeight * Scale;

    // 绘制每个 Stage
    for (AStage* Stage : AllStages)
    {
        if (!Stage) continue;

        if (Settings->bDetailedMode)
        {
            DrawStageDetailed(Stage, YOffset);
        }
        else
        {
            DrawStageSimple(Stage, YOffset);
        }
    }
}

FVector2D AStageDebugHUD::GetStartPosition() const
{
    UStageDebugSettings* Settings = UStageDebugSettings::Get();
    if (!Settings) return FVector2D(50.0f, 50.0f);

    const float ScreenWidth = Canvas ? Canvas->SizeX : 1920.0f;
    const float ScreenHeight = Canvas ? Canvas->SizeY : 1080.0f;
    const float Margin = 50.0f;

    switch (Settings->DisplayPosition)
    {
    case EStageDebugPosition::TopLeft:
        return FVector2D(Margin, Margin);
    case EStageDebugPosition::TopRight:
        return FVector2D(ScreenWidth - 400.0f, Margin);
    case EStageDebugPosition::BottomLeft:
        return FVector2D(Margin, ScreenHeight - 300.0f);
    case EStageDebugPosition::BottomRight:
        return FVector2D(ScreenWidth - 400.0f, ScreenHeight - 300.0f);
    case EStageDebugPosition::Custom:
        return Settings->CustomOffset;
    default:
        return FVector2D(Margin, Margin);
    }
}

void AStageDebugHUD::DrawStageSimple(AStage* Stage, float& YOffset)
{
    UStageDebugSettings* Settings = UStageDebugSettings::Get();
    const float Scale = Settings ? Settings->TextScale : 1.0f;
    const FVector2D StartPos = GetStartPosition();

    FString Text = FString::Printf(TEXT("%s: %s | DL: %s"),
        *Stage->GetActorLabel(),
        *StateToString(Stage->GetCurrentStageState()),
        *DataLayerStateToString(Stage->GetStageDataLayerState()));

    FLinearColor Color = GetStateColor(Stage->GetCurrentStageState());
    DrawText(Text, Color, StartPos.X, YOffset, nullptr, Scale);
    YOffset += LineHeight * Scale;
}

void AStageDebugHUD::DrawStageDetailed(AStage* Stage, float& YOffset)
{
    UStageDebugSettings* Settings = UStageDebugSettings::Get();
    const float Scale = Settings ? Settings->TextScale : 1.0f;
    const FVector2D StartPos = GetStartPosition();
    const float Indent = IndentWidth * Scale;

    FLinearColor StateColor = GetStateColor(Stage->GetCurrentStageState());

    // Stage 名称和 ID
    FString Header = FString::Printf(TEXT("%s (ID:%d)"),
        *Stage->GetActorLabel(), Stage->GetStageID());
    DrawText(Header, StateColor, StartPos.X, YOffset, nullptr, Scale);
    YOffset += LineHeight * Scale;

    // State
    FString StateText = FString::Printf(TEXT("├─ State: %s%s"),
        *StateToString(Stage->GetCurrentStageState()),
        Stage->IsStageStateLocked() ? TEXT(" (Locked)") : TEXT(""));
    DrawText(StateText, FLinearColor::White, StartPos.X + Indent, YOffset, nullptr, Scale * 0.9f);
    YOffset += LineHeight * Scale;

    // DataLayer
    FString DLText = FString::Printf(TEXT("├─ StageDataLayer: %s"),
        *DataLayerStateToString(Stage->GetStageDataLayerState()));
    DrawText(DLText, FLinearColor::White, StartPos.X + Indent, YOffset, nullptr, Scale * 0.9f);
    YOffset += LineHeight * Scale;

    // Active Acts
    TArray<int32> ActiveActs = Stage->GetActiveActIDs();
    TArray<int32> AllActs = Stage->GetAllActIDs();
    FString ActsStr;
    for (int32 ActID : AllActs)
    {
        if (!ActsStr.IsEmpty()) ActsStr += TEXT(", ");
        ActsStr += FString::Printf(TEXT("%d%s"), ActID, ActiveActs.Contains(ActID) ? TEXT("✓") : TEXT(""));
    }
    FString ActsText = FString::Printf(TEXT("├─ Acts: [%s]"), *ActsStr);
    DrawText(ActsText, FLinearColor::White, StartPos.X + Indent, YOffset, nullptr, Scale * 0.9f);
    YOffset += LineHeight * Scale;

    // Zone Actor Counts
    FString LoadZoneText = FString::Printf(TEXT("├─ LoadZone: %d actors"),
        Stage->OverlappingLoadZoneActors.Num());
    DrawText(LoadZoneText, FLinearColor::White, StartPos.X + Indent, YOffset, nullptr, Scale * 0.9f);
    YOffset += LineHeight * Scale;

    FString ActivateZoneText = FString::Printf(TEXT("└─ ActivateZone: %d actors"),
        Stage->OverlappingActivateZoneActors.Num());
    DrawText(ActivateZoneText, FLinearColor::White, StartPos.X + Indent, YOffset, nullptr, Scale * 0.9f);
    YOffset += LineHeight * Scale;

    // 空行分隔
    YOffset += LineHeight * Scale * 0.5f;
}

FLinearColor AStageDebugHUD::GetStateColor(EStageRuntimeState State) const
{
    switch (State)
    {
    case EStageRuntimeState::Unloaded:   return FLinearColor::Gray;
    case EStageRuntimeState::Preloading: return FLinearColor::Yellow;
    case EStageRuntimeState::Loaded:     return FLinearColor(0.5f, 0.8f, 1.0f); // Light Blue
    case EStageRuntimeState::Active:     return FLinearColor::Green;
    case EStageRuntimeState::Unloading:  return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
    default:                             return FLinearColor::White;
    }
}

FLinearColor AStageDebugHUD::GetDataLayerStateColor(EDataLayerRuntimeState State) const
{
    switch (State)
    {
    case EDataLayerRuntimeState::Unloaded:  return FLinearColor::Gray;
    case EDataLayerRuntimeState::Loaded:    return FLinearColor(0.5f, 0.8f, 1.0f);
    case EDataLayerRuntimeState::Activated: return FLinearColor::Green;
    default:                                return FLinearColor::White;
    }
}

FString AStageDebugHUD::StateToString(EStageRuntimeState State)
{
    switch (State)
    {
    case EStageRuntimeState::Unloaded:   return TEXT("Unloaded");
    case EStageRuntimeState::Preloading: return TEXT("Preloading");
    case EStageRuntimeState::Loaded:     return TEXT("Loaded");
    case EStageRuntimeState::Active:     return TEXT("Active");
    case EStageRuntimeState::Unloading:  return TEXT("Unloading");
    default:                             return TEXT("Unknown");
    }
}

FString AStageDebugHUD::DataLayerStateToString(EDataLayerRuntimeState State)
{
    switch (State)
    {
    case EDataLayerRuntimeState::Unloaded:  return TEXT("Unloaded");
    case EDataLayerRuntimeState::Loaded:    return TEXT("Loaded");
    case EDataLayerRuntimeState::Activated: return TEXT("Activated");
    default:                                return TEXT("Unknown");
    }
}
```

### 4.5 控制台命令注册

在 `StageEditorRuntimeModule.cpp` 或新建文件中：

```cpp
#include "Debug/StageDebugSettings.h"

// 控制台命令
static FAutoConsoleCommand StageDebugCommand(
    TEXT("Stage.Debug"),
    TEXT("Toggle Stage Debug HUD. Usage: Stage.Debug 0|1"),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
    {
        UStageDebugSettings* Settings = UStageDebugSettings::Get();
        if (!Settings) return;

        if (Args.Num() > 0)
        {
            Settings->bEnableDebugHUD = FCString::Atoi(*Args[0]) != 0;
        }
        else
        {
            // Toggle
            Settings->bEnableDebugHUD = !Settings->bEnableDebugHUD;
        }

        UE_LOG(LogTemp, Log, TEXT("Stage Debug HUD: %s"),
            Settings->bEnableDebugHUD ? TEXT("Enabled") : TEXT("Disabled"));
    })
);
```

### 4.6 Build.cs 依赖

确保 `StageEditorRuntime.Build.cs` 包含：

```csharp
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core",
    "CoreUObject",
    "Engine",
    "Slate",
    "SlateCore",
    "DeveloperSettings",  // ← 确保有这个
    // ...
});
```

---

## 5. 使用 HUD 的方式

### 5.1 方式一：通过 GameMode 设置

```cpp
// 在你的 GameMode 中
AYourGameMode::AYourGameMode()
{
    HUDClass = AStageDebugHUD::StaticClass();
}
```

### 5.2 方式二：运行时 Spawn

```cpp
// 在某处 Spawn HUD（如 PlayerController）
if (AStageDebugHUD* DebugHUD = GetWorld()->SpawnActor<AStageDebugHUD>())
{
    // HUD 已创建
}
```

### 5.3 方式三：蓝图配置

在 GameMode 蓝图中设置 HUD Class 为 `StageDebugHUD`。

---

## 6. 测试方法

1. **编译项目**
2. **设置 HUD**: GameMode 中设置 HUDClass = AStageDebugHUD
3. **开启 Debug**: 控制台输入 `Stage.Debug 1`
4. **验证显示**: PIE 运行，查看屏幕左上角是否显示 Stage 信息
5. **测试位置**: Project Settings → Plugins → Stage Editor → 修改 DisplayPosition
6. **测试模式切换**: 修改 bDetailedMode 查看简洁/详细模式

---

## 7. 注意事项

1. **EStageRuntimeState** 定义在 `Core/StageCoreTypes.h`，需要 include
2. **EDataLayerRuntimeState** 来自 UE 引擎，在 `WorldPartition/DataLayer/DataLayerInstance.h`
3. **OverlappingLoadZoneActors** 和 **OverlappingActivateZoneActors** 是 Stage 的 public 成员
4. **UDeveloperSettings** 的 `config = Game` 会保存到 DefaultGame.ini
5. **控制台命令** 修改 Settings 后不会自动保存，需要用户手动保存或下次启动时重置

---

## 8. 相关文档

- `Docs/StageEditor/ImplementationPlan/FutureTodoList.md` - 完整需求列表
- `Docs/StageEditor/StageManagerSubsystem.md` - Subsystem API 文档
- `CLAUDE.md` - 项目整体说明

---

## 9. 编译命令

```bat
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ExtendEditorEditor Win64 Development -Project="D:\UEProject\ReserchPJ\ExtendEditor\ExtendEditor\ExtendEditor.uproject" -WaitMutex
```
