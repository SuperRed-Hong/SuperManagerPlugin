# SSplitter ResizeMode 深度解析

> **创建日期:** 2025-12-05
> **适用版本:** Unreal Engine 5.6
> **相关问题:** SStageDataLayerOutliner 列宽调整方向反转

---

## 📋 问题背景

在 SStageDataLayerOutliner 中使用 `ManualWidth` 模式配置列宽时，发现拖动列分隔线的行为与原生 SceneOutliner 相反：

**期望行为:**
- 往左拖拽分隔线 → 左列变窄，右列变宽
- 往右拖拽分隔线 → 左列变宽，右列变窄

**实际行为:**
- 往左拖拽分隔线 → 右列变窄，左列变宽（反向）
- 所有列的分隔线都有这个问题

---

## 🎯 核心原理：SSplitter 是什么？

### 定义

`SSplitter` 是 Slate UI 框架中的容器控件，用于将多个子控件（slots）排列在一起，并在它们之间插入**可拖动的分隔线**。

### 基本用法

```cpp
SNew(SSplitter)
.Orientation(Orient_Horizontal)  // 水平排列
.ResizeMode(ESplitterResizeMode::Fill)  // 调整模式
+ SSplitter::Slot()  // 第一个 slot
[
    SNew(STextBlock).Text(LOCTEXT("Left", "左边"))
]
+ SSplitter::Slot()  // 第二个 slot
[
    SNew(STextBlock).Text(LOCTEXT("Right", "右边"))
]
```

**视觉效果:**
```
┌─────────────│──────────────┐
│   左边      │    右边      │
│   Slot 0    │    Slot 1    │
└─────────────┴──────────────┘
         ↑
    可拖动的分隔线
```

---

## 🔧 三种 ResizeMode 详解

### 1. FixedPosition 模式

**定义:**
```cpp
namespace ESplitterResizeMode
{
    /** Resize the selected slot. If space is needed, then resize the next resizable slot. */
    FixedPosition,
}
```

**行为:**
- 拖动分隔线时，调整**被拖动位置左侧的 slot**
- 如果需要额外空间，调整**下一个可调整大小的 slot**
- 其他 slot 保持不变

**示例:**
```
初始状态: [A:200px] | [B:200px] | [C:200px]
拖动 A|B 分隔线往右 50px:
结果:     [A:250px] | [B:150px] | [C:200px]
          ↑ 增加    ↑ 减少      ↑ 不变
```

---

### 2. FixedSize 模式

**定义:**
```cpp
namespace ESplitterResizeMode
{
    /** Resize the selected slot. If space is needed, then resize the last resizable slot. */
    FixedSize,
}
```

**行为:**
- 拖动分隔线时，调整**被拖动位置左侧的 slot**
- 如果需要额外空间，调整**最后一个可调整大小的 slot**
- 中间的 slot 保持不变

**示例:**
```
初始状态: [A:200px] | [B:200px] | [C:200px]
拖动 A|B 分隔线往右 50px:
结果:     [A:250px] | [B:200px] | [C:150px]
          ↑ 增加    ↑ 不变      ↑ 减少（最后一个）
```

---

### 3. Fill 模式（⭐ 推荐）

**定义:**
```cpp
namespace ESplitterResizeMode
{
    /** Resize the selected slot by redistributing the available space with the following resizable slots. */
    Fill,
}
```

**行为:**
- 使用**比例（fraction）**而非固定像素值
- 拖动分隔线时，重新分配总宽度
- 所有后续可调整 slot **按比例**共享变化

**核心机制: 比例分配**

每个 slot 的 `Value` 属性表示占总宽度的比例（0.0 ~ 1.0）：

```cpp
Splitter->AddSlot()
    .Value(0.5)  // 占 50% 宽度
    .SizeRule(SSplitter::FractionOfParent)  // 使用比例模式
    [Widget]
```

**详细示例:**

```
总宽度: 600px

初始状态:
┌────────────────┬────────┬────────┐
│  Label (0.6)   │ SUID   │Actions │
│  360px         │(0.2)   │(0.2)   │
│                │120px   │120px   │
└────────────────┴────────┴────────┘

拖动 Label|SUID 分隔线往左 60px:

步骤 1: 计算 Label 新宽度
OldWidth = 360px
NewWidth = 360 - 60 = 300px

步骤 2: 计算新比例
NewFraction = 300 / 600 = 0.5

步骤 3: 更新 Label 的 Value
Label.Value = 0.5 (从 0.6 变为 0.5)

步骤 4: 重新分配释放的空间
Released = 0.6 - 0.5 = 0.1

SUID 和 Actions 的原比例: 0.2 : 0.2 = 1:1
按比例分配 0.1:
SUID.Value   = 0.2 + 0.05 = 0.25
Actions.Value = 0.2 + 0.05 = 0.25

结果:
┌──────────────┬──────────┬──────────┐
│ Label (0.5)  │SUID(0.25)│Actions   │
│ 300px        │ 150px    │(0.25)    │
│              │          │150px     │
└──────────────┴──────────┴──────────┘
  ↓ 减少 60px   ↑ 增加 30px ↑ 增加 30px

总和仍然是 1.0 (0.5 + 0.25 + 0.25)
```

---

## 🚫 Manual 模式的问题

### SHeaderRow 中的 Manual 模式实现

Manual 模式不是 SSplitter 的三种模式之一，而是 **SHeaderRow 自定义实现**的一种列调整方式。

**关键问题: 双 Slot 结构**

Manual 模式为每列创建 **TWO** 个 SSplitter slots:

```cpp
// SHeaderRow.cpp - Manual 模式实现
case EColumnSizeMode::Manual:
{
    // ❌ Slot 1: 列内容（不可调整大小）
    Splitter->AddSlot()
        .SizeRule(SSplitter::SizeToContent)
        .Resizable(false)  // 不可调整
        [
            SNew(SBox)
            .WidthOverride(列.GetWidth())  // 固定宽度
            [列内容]
        ]

    // ❌ Slot 2: SizingGrip（可调整大小的手柄）
    Splitter->AddSlot()
        .SizeRule(SSplitter::SizeToContent)
        .Resizable(true)   // 可调整
        .OnSlotResized([&列](float NewValue) {
            列.SetWidth(NewValue);  // 更新列宽
        })
        [
            SNew(SBorder)  // SizingGrip - 拖动手柄
            .Cursor(EMouseCursor::ResizeLeftRight)
            .OnMouseMove([&列](const FGeometry&, const FPointerEvent& Event) {
                // 直接计算鼠标移动 delta
                float NewWidth = 列.GetWidth() + Event.GetCursorDelta().X;
                列.SetWidth(FMath::Max(20.0f, NewWidth));
            })
        ]
}
```

### 实际 Slot 布局对比

**Fill 模式（正常）:**
```
视觉:  ┌──────────┬────────┬──────────┐
       │  Label   │  SUID  │ Actions  │
       └──────────┴────────┴──────────┘

Slots: ┌──────────┬────────┬──────────┐
       │ Slot 0   │ Slot 1 │ Slot 2   │
       └──────────┴────────┴──────────┘

一一对应，索引清晰
```

**Manual 模式（混乱）:**
```
视觉:  ┌──────────┬────────┬──────────┐
       │  Label   │  SUID  │ Actions  │
       └──────────┴────────┴──────────┘

Slots: ┌──────────┬──┬────┬──┬────────┬──┐
       │Label内容 │G │SUID│G │Actions │G │  ← G = SizingGrip
       │ Slot 0   │1 │  2 │3 │   4    │5 │
       └──────────┴──┴────┴──┴────────┴──┘

每列变成 2 个 slots，索引错位！
```

### 方向反转的根本原因

```
用户操作: 拖动 SUID 和 Actions 之间的视觉分隔线
期望:     调整 SUID 或 Actions 的宽度

实际发生:
1. 用户以为拖的是 Slot 2 (SUID) 和 Slot 4 (Actions) 之间
2. 但实际拖的是 Slot 3 (SUID的Grip) 和 Slot 4 (Actions内容) 之间
3. SSplitter 认为你在调整 Slot 3 的大小
4. Slot 3 的 OnSlotResized 被触发
5. 这个回调绑定的是 SUID.SetWidth()
6. 结果: 拖动 SUID 右边的线，改变的是 SUID 的宽度

方向反转:
- 往左拖 → Slot 3 (Grip) 变小 → SUID 的 Grip 变小
- 但 SizingGrip 的 OnMouseMove 会计算 CursorDelta.X
- Delta.X < 0 (往左) → SUID.Width 减少
- 同时 SSplitter 为了给 Slot 3 腾空间，会扩大 Slot 2 (SUID内容)
- 两个效果叠加 → 视觉上感觉方向反了
```

---

## ✅ 解决方案: 使用 Fill 模式

### 方案原理

1. **移除 Manual 模式**，改用 Fill 模式
2. 通过 `FSceneOutlinerColumnInfo` 的 `FillSize` 参数指定列宽比例
3. 在列的 `ConstructHeaderRowColumn()` 中**不指定宽度模式**
4. SceneOutliner 自动将 FillSize 转换为 `FillWidth()`

### 实现步骤

#### Step 1: 在 ColumnInfo 注册时指定 FillSize

**文件:** `SStageDataLayerOutliner.cpp`

```cpp
// Label 列 - 自动填充剩余空间
InitOptions.ColumnMap.Add(
    FSceneOutlinerBuiltInColumnTypes::Label(),
    FSceneOutlinerColumnInfo(
        ESceneOutlinerColumnVisibility::Visible,
        3,
        FCreateSceneOutlinerColumn(),
        true,
        1.0f  // ⭐ FillSize = 1.0 (占剩余空间的 100%)
    )
);

// SUID 列 - 固定比例
InitOptions.ColumnMap.Add(
    FStageDataLayerSUIDColumn::GetID(),
    FSceneOutlinerColumnInfo(
        ESceneOutlinerColumnVisibility::Visible,
        4,
        FCreateSceneOutlinerColumn::CreateLambda([](ISceneOutliner& Outliner) {
            return MakeShared<FStageDataLayerSUIDColumn>(Outliner);
        }),
        true,
        0.1f  // ⭐ FillSize = 0.1 (占总宽度的 10%)
    )
);

// Actions 列 - 固定比例
InitOptions.ColumnMap.Add(
    FStageDataLayerActionsColumn::GetID(),
    FSceneOutlinerColumnInfo(
        ESceneOutlinerColumnVisibility::Visible,
        5,
        FCreateSceneOutlinerColumn::CreateLambda([](ISceneOutliner& Outliner) {
            return MakeShared<FStageDataLayerActionsColumn>(Outliner);
        }),
        true,
        0.2f  // ⭐ FillSize = 0.2 (占总宽度的 20%)
    )
);
```

#### Step 2: 移除列中的 ManualWidth

**文件:** `StageDataLayerColumns.cpp`

```cpp
// SUID 列
SHeaderRow::FColumn::FArguments FStageDataLayerSUIDColumn::ConstructHeaderRowColumn()
{
    return SHeaderRow::Column(GetColumnID())
        // ❌ 移除: .ManualWidth(60.f)
        // ✅ 不指定宽度，SceneOutliner 会自动应用 FillWidth(0.1)
        .HAlignHeader(HAlign_Center)
        .VAlignHeader(VAlign_Center)
        .HAlignCell(HAlign_Center)
        .VAlignCell(VAlign_Center)
        .DefaultTooltip(LOCTEXT("SUIDColumnTooltip", "Stage Unique ID"))
        [
            SNew(STextBlock)
            .Text(LOCTEXT("SUIDHeader", "SUID"))
        ];
}

// Actions 列
SHeaderRow::FColumn::FArguments FStageDataLayerActionsColumn::ConstructHeaderRowColumn()
{
    return SHeaderRow::Column(GetColumnID())
        // ❌ 移除: .ManualWidth(160.f)
        // ✅ 不指定宽度，SceneOutliner 会自动应用 FillWidth(0.2)
        .HAlignHeader(HAlign_Center)
        .VAlignHeader(VAlign_Center)
        .HAlignCell(HAlign_Left)
        .VAlignCell(VAlign_Center)
        .DefaultTooltip(LOCTEXT("ActionsColumnTooltip", "Actions"))
        [
            SNew(STextBlock)
            .Text(LOCTEXT("ActionsHeader", "Actions"))
        ];
}
```

### 自动应用机制

SceneOutliner 在创建 HeaderRow 时会自动处理 FillSize：

```cpp
// SSceneOutliner.cpp
if (ColumnInfo.FillSize.IsSet())
{
    ColumnArgs.FillWidth(ColumnInfo.FillSize.GetValue());
}
```

最终传递给 SHeaderRow 的就是 `.FillWidth(0.1)` 或 `.FillWidth(0.2)`。

---

## 📊 对比总结

| 特性 | Fill 模式 | Manual 模式 |
|------|-----------|-------------|
| **Splitter Slots 数量** | 每列 1 个 | 每列 2 个（内容+Grip） |
| **Slot 索引对应** | 直接对应列索引 | 错位（Grip 占用索引） |
| **拖动方向** | ✅ 正确（符合直觉） | ❌ 反向（混乱） |
| **宽度单位** | 比例（0.0~1.0） | 像素（固定值） |
| **窗口缩放** | ✅ 自动按比例调整 | ❌ 需手动计算 |
| **最小宽度保护** | ✅ SSplitter 内置 | ⚠️ 需自定义逻辑 |
| **配置方式** | `FillSize` 参数 | `ManualWidth()` 调用 |
| **UE 原生使用** | ✅ SceneOutliner | ❌ 罕见（仅特殊场景） |
| **代码复杂度** | ✅ 简单（委托给框架） | ❌ 复杂（自定义 Grip） |

---

## 🎯 最佳实践

### 1. 优先使用 Fill 模式

对于大多数列布局场景，Fill 模式是最佳选择：
- 代码简洁
- 行为直觉
- 自适应窗口
- 与原生一致

### 2. FillSize 的分配策略

```cpp
// 推荐的比例分配
固定功能列（图标、按钮）: 0.05 ~ 0.15
次要信息列（ID、状态）:   0.1  ~ 0.2
主要内容列（Label、名称）: 1.0（自动填充剩余）

// 示例
Visibility:      (FixedWidth 24px - 不参与比例)
LoadedInEditor:  (FixedWidth 24px - 不参与比例)
SyncStatus:      (FixedWidth 24px - 不参与比例)
Label:           FillSize = 1.0   (填充剩余空间)
SUID:            FillSize = 0.1   (占总宽 10%)
Actions:         FillSize = 0.2   (占总宽 20%)
```

### 3. FixedWidth 的使用场景

仅用于**纯图标列**或**不需要调整的列**：
```cpp
.FixedWidth(24.f)  // Eye icon
.FixedWidth(24.f)  // Checkbox
```

### 4. 避免使用 ManualWidth

除非有**极特殊需求**（如需要像素级精确控制且不需要拖动调整），否则不推荐使用。

---

## 🔍 调试技巧

### 查看 Slot 布局

如果遇到调整方向问题，可以在代码中打印 Splitter slots 信息：

```cpp
// 在 SSceneOutliner 或 SHeaderRow 中添加调试代码
void DebugPrintSlots(TSharedPtr<SSplitter> Splitter)
{
    if (!Splitter.IsValid()) return;

    UE_LOG(LogTemp, Warning, TEXT("=== Splitter Slots Debug ==="));

    int32 SlotIndex = 0;
    for (int32 i = 0; i < Splitter->GetChildren()->Num(); ++i)
    {
        auto& Slot = (*Splitter->GetChildren())[i];

        UE_LOG(LogTemp, Warning, TEXT("Slot %d: Resizable=%s, SizeRule=%d"),
            i,
            Slot.CanBeResized() ? TEXT("Yes") : TEXT("No"),
            (int32)Slot.GetSizeRule()
        );
    }
}
```

**正常输出（Fill 模式）:**
```
Slot 0: Resizable=Yes, SizeRule=FractionOfParent
Slot 1: Resizable=Yes, SizeRule=FractionOfParent
Slot 2: Resizable=Yes, SizeRule=FractionOfParent
```

**异常输出（Manual 模式）:**
```
Slot 0: Resizable=No,  SizeRule=SizeToContent
Slot 1: Resizable=Yes, SizeRule=SizeToContent  (Grip)
Slot 2: Resizable=No,  SizeRule=SizeToContent
Slot 3: Resizable=Yes, SizeRule=SizeToContent  (Grip)
...
```

---

## 📚 参考资源

**引擎源码:**
- `Engine/Source/Runtime/Slate/Public/Widgets/Layout/SSplitter.h`
- `Engine/Source/Runtime/Slate/Private/Widgets/Layout/SSplitter.cpp`
- `Engine/Source/Runtime/Slate/Public/Widgets/Views/SHeaderRow.h`
- `Engine/Source/Runtime/Slate/Private/Widgets/Views/SHeaderRow.cpp`
- `Engine/Source/Editor/SceneOutliner/Public/SceneOutlinerPublicTypes.h`
- `Engine/Source/Editor/SceneOutliner/Private/SSceneOutliner.cpp`

**相关文档:**
- [StageEditor Overview](../Overview.md)
- [Phase 8.2 - Custom Columns Implementation](../DataLayerIntegration/Phase8_2_CustomColumns.md)

---

*文档创建: 2025-12-05*
*最后更新: 2025-12-05*
