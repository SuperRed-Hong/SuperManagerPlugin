# StageEditor World Partition 强制要求使用指南

## 概述

从当前版本开始，**StageEditor 要求关卡必须是 World Partition Level** 才能正常使用。这是因为 StageEditor 的核心功能（Act 切换、动态资源加载）依赖于 World Partition 的 DataLayer 流式加载机制。

---

## 用户体验

### 场景 1：打开 World Partition Level ✅

当你在一个 World Partition 关卡中打开 StageEditor 时：

```
Tools → Stage Editor
```

**UI 显示：**
- ✅ 完整的 StageEditor 界面
- ✅ 所有功能按钮可用（Register Props, Create Act, etc.）
- ✅ 树形视图正常显示
- ✅ 可以正常使用所有功能

---

### 场景 2：打开传统 Level ⚠️

当你在一个传统关卡中打开 StageEditor 时：

```
Tools → Stage Editor
```

**UI 显示：**

```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│          ⚠ World Partition Required                    │
│                                                         │
│  StageEditor requires a World Partition level to       │
│  function properly.                                     │
│                                                         │
│  World Partition enables:                              │
│  • Dynamic resource streaming via DataLayers           │
│  • Efficient memory management for large worlds        │
│  • Proper Act-based scene state management             │
│                                                         │
│  Please convert your current level to World Partition  │
│  to continue.                                           │
│                                                         │
│         [Convert to World Partition]                    │
│                                                         │
│  Learn more about World Partition in UE5 documentation │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

**特点：**
- ❌ 所有编辑功能不可用
- ✅ 显示清晰的警告信息
- ✅ 提供一键转换按钮
- ✅ 说明为什么需要 World Partition

---

## 一键转换功能

### 使用步骤

1. **点击转换按钮**
   - 在警告界面中点击 `[Convert to World Partition]`

2. **确认对话框**
   ```
   Convert to World Partition?

   This will convert the current level to World Partition format.

   ⚠ Important:
   • This operation cannot be undone
   • The level will be saved and reloaded
   • Make sure you have saved all your work

   Do you want to continue?

   [Yes]  [No]
   ```

3. **转换过程**
   - ✅ 保存当前关卡
   - ✅ 创建 World Partition 结构
   - ✅ 自动重载关卡
   - ✅ StageEditor 自动切换到正常模式

4. **完成**
   ```
   ✅ Successfully converted to World Partition!
   ✅ Conversion complete! StageEditor is now ready to use.
   ```

---

## 转换原理

### 技术实现

```cpp
// StageEditorController.cpp:789
UWorldPartition* NewWorldPartition = UWorldPartition::CreateOrRepairWorldPartition(WorldSettings);
```

**转换步骤：**
1. 检查当前关卡是否已经是 World Partition
2. 保存关卡（确保数据不丢失）
3. 调用 `CreateOrRepairWorldPartition` 创建 WP 结构
4. 初始化 World Partition
5. 保存并重载关卡

**安全措施：**
- ✅ 转换前自动保存
- ✅ 转换失败时保留原始关卡
- ✅ 提供明确的错误提示
- ✅ 不会丢失任何 Actor 或数据

---

## 手动转换（备用方案）

如果一键转换失败，可以使用 UE5 的内置功能：

### 方法 1：主菜单转换

```
Window → World Settings → Enable World Partition
```

### 方法 2：创建新的 World Partition Level

1. `File → New Level`
2. 选择 `Open World` 模板（自带 World Partition）
3. 将原始关卡的内容复制过来

---

## 检测逻辑

### 在代码中检测 World Partition

```cpp
// StageEditorController.h:102
bool IsWorldPartitionActive() const;

// StageEditorController.cpp:748
bool FStageEditorController::IsWorldPartitionActive() const
{
    if (UWorld* World = GEditor->GetEditorWorldContext().World())
    {
        return World->GetWorldPartition() != nullptr;
    }
    return false;
}
```

### UI 自动切换

```cpp
// StageEditorPanel.cpp:42
const bool bIsWorldPartition = IsWorldPartitionLevel();

if (!bIsWorldPartition)
{
    // Show warning UI with conversion button
}
else
{
    // Show normal StageEditor UI
}
```

---

## 用户常见问题

### Q: 为什么必须使用 World Partition？

**A:** StageEditor 的核心功能依赖 World Partition 的 DataLayer 系统：

| 功能 | 需要 World Partition | 原因 |
|------|---------------------|------|
| Act 切换时的资源加载/卸载 | ✅ 是 | DataLayer 的流式加载 |
| 大型场景的内存优化 | ✅ 是 | 自动分块管理 |
| 动态场景状态管理 | ✅ 是 | Runtime 状态切换 |

在传统 Level 中，DataLayer 只能控制**可见性**，无法实现真正的**资源加载**。

---

### Q: 转换会丢失我的数据吗？

**A:** 不会。转换过程：
- ✅ 保留所有 Actor
- ✅ 保留所有组件和属性
- ✅ 保留 Lighting 和其他设置
- ✅ 只是添加 World Partition 结构

**唯一变化：**
- Level 文件结构从单文件变为分块文件
- 多了一个 `_WP` 文件夹用于存储 World Partition 数据

---

### Q: 转换后能恢复吗？

**A:** 技术上可以，但不推荐：
- ⚠️ 需要手动删除 World Partition 结构
- ⚠️ 可能需要重新保存关卡
- ✅ 建议：转换前备份关卡文件

**最佳实践：**
在转换前使用版本控制（Git）创建一个提交点。

---

### Q: 如果转换失败怎么办？

**A:** 转换失败时的处理：

1. **查看错误日志**
   ```
   Error: Failed to create World Partition.
   Please convert manually via: World > Enable World Partition
   ```

2. **使用手动转换**
   - `Window → World Settings → Enable World Partition`

3. **联系支持**
   - 提供错误日志
   - 描述关卡的复杂度（Actor 数量、Sub-Levels 等）

---

## 开发者信息

### 相关代码文件

**UI 层：**
- `StageEditorPanel.h:140` - `IsWorldPartitionLevel()` 检测方法
- `StageEditorPanel.cpp:42-221` - 条件 UI 渲染
- `StageEditorPanel.cpp:1638-1667` - 转换按钮回调

**逻辑层：**
- `StageEditorController.h:102-106` - WP 检测和转换接口
- `StageEditorController.cpp:748-750` - WP 检测实现
- `StageEditorController.cpp:751-818` - WP 转换实现

### 关键 API

```cpp
// 检测 World Partition
UWorld::GetWorldPartition() -> UWorldPartition*

// 创建 World Partition
UWorldPartition::CreateOrRepairWorldPartition(AWorldSettings*) -> UWorldPartition*

// 初始化
UWorldPartition::Initialize(UWorld*, FTransform)
```

---

## 总结

### 用户视角

| 关卡类型 | StageEditor 行为 | 操作 |
|---------|-----------------|------|
| World Partition Level | ✅ 完全可用 | 正常使用 |
| 传统 Level | ❌ 功能禁用 | 点击转换按钮 |

### 设计优势

1. ✅ **清晰的要求**：用户立即知道需要 World Partition
2. ✅ **便捷的转换**：一键转换，无需手动操作
3. ✅ **安全的流程**：自动保存，防止数据丢失
4. ✅ **友好的提示**：说明为什么需要 WP
5. ✅ **专业的体验**：强制使用正确的工具链

---

## 更新日志

**Version 1.0 (2025-01-21)**
- ✅ 添加 World Partition 强制要求
- ✅ 实现一键转换功能
- ✅ 添加条件 UI 显示
- ✅ 完善错误处理和用户提示
