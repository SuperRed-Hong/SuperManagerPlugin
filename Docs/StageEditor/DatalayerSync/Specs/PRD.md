# DataLayer 导入功能 - 产品需求文档 (PRD)

> 创建日期: 2025-11-29
> 状态: ⚠️ 部分实施（UI 功能待完善）
> 开发日志: [../DevLog/Overview.md](../DevLog/Overview.md)
> UI 详细设计: [../UI_Development.md](../UI_Development.md)

---

## 1. 功能概述

### 1.1 背景

用户在未安装 StageEditor 插件时，可能已经通过手动方式布设了一系列 DataLayer，并在这些 DataLayer 中注册了许多 Actor。

### 1.2 目标

让 StageEditor 插件能够根据已有的 DataLayer 数据（名称、层级、成员 Actor），通过算法自动匹配并创建 Stage-Act-Prop 架构，实现**项目中途接管**能力。

### 1.3 核心价值

| 价值 | 说明 |
|------|------|
| **降低迁移成本** | 无需重建 DataLayer，直接复用现有结构 |
| **提高工作效率** | 一键导入，自动识别层级关系 |
| **保持数据同步** | 实时监测 DataLayer 变化，提示用户同步 |
| **增量导入支持** | 支持分批导入，已导入的不重复处理 |

---

## 2. 用户场景

### 2.1 主要场景

**场景：项目中途接入**

用户已经使用 UE 原生 DataLayer 系统管理场景结构，现在希望迁移到 StageEditor 以获得更强的状态管理能力。

**用户期望：**
1. 按照规范重命名现有 DataLayer
2. 一键将 DataLayer 结构导入为 Stage-Act-Prop 架构
3. 后续新增的 DataLayer 能被自动识别并提示同步

---

## 3. DataLayer 命名规范

### 3.1 标准命名格式

```
Stage DataLayer:  DL_Stage_{StageName}
Act DataLayer:    DL_Act_{StageName}_{ActName}
```

### 3.2 示例

```
DL_Stage_Castle                    ← Stage 级别
├── DL_Act_Castle_Exterior         ← Act 级别（必须为 Stage 的子层）
├── DL_Act_Castle_GreatHall
└── DL_Act_Castle_Dungeon
```

### 3.3 规则说明

| 规则 | 说明 |
|------|------|
| Stage 前缀 | `DL_Stage_` |
| Act 前缀 | `DL_Act_` |
| 不包含数字 ID | SUID 由系统内部管理，命名纯语义化 |
| **Act 必须为 Stage 子层** | 父子层级关系强制要求 |

---

## 4. UI 设计

> **详细文档**: [../UI_Development.md](../UI_Development.md)

### 4.1 概述

Stage DataLayer Browser 提供 DataLayer 导入和同步的可视化界面。

**目标 UI 布局**：

```
┌─────────────┬──────────────────────┬────────────┬──────────────┐
│ Sync Status │ Item Label           │ SUID       │ Actions      │
├─────────────┼──────────────────────┼────────────┼──────────────┤
│ ✓ (绿色)    │ DL_Stage_Castle      │ S:1        │ [Sync]       │
│ ⚠ (橙色)    │ DL_Act_Castle_Ext    │ A:1.2      │ [Sync]       │
│ ● (蓝色)    │ DL_Stage_Forest      │            │ [Import]     │
└─────────────┴──────────────────────┴────────────┴──────────────┘
```

### 4.2 核心 UI 组件

| 组件 | 说明 | 状态 |
|------|------|------|
| **工具栏** | Sync All、Import Selected 按钮 | ✅ 已实现 |
| **Sync Status 列** | 同步状态图标（绿/橙/蓝） | ⏳ 待实现 |
| **SUID 列** | 显示 Stage/Act ID（S:1, A:1.2） | ⏳ 待实现 |
| **Actions 列** | 行内 Import/Sync 按钮 | ⏳ 待实现 |
| **导入预览对话框** | 显示导入预览和统计 | ✅ 已实现 |

### 4.3 当前实现状态

当前采用**包装器方案**，存在以下限制：
- 无法添加自定义列（Sync Status、SUID、Actions）
- 无法获取内嵌 Outliner 的选中项

**改进方案**正在讨论中，详见 [UI_Development.md](../UI_Development.md) 第 4 节。

---

## 5. 同步状态

| 状态 | 图标颜色 | 说明 |
|------|---------|------|
| **已同步 (Synced)** | 绿色 | DataLayer 与 Stage 数据一致 |
| **不同步 (OutOfSync)** | 橙色 | 检测到变化，需要同步 |
| **未导入 (NotImported)** | 蓝色 | 尚未导入到 StageEditor |

### 不同步触发条件

- 新增 Act DataLayer
- 删除 Act DataLayer
- Actor 成员变化

---

## 6. 设计决策汇总

| 决策点 | 选择 | 原因 |
|--------|------|------|
| 命名规范 | `DL_Stage_{Name}` / `DL_Act_{Stage}_{Name}` | 语义清晰 |
| 元数据存储方案 | 反向查找 | 简化架构，无需额外存储 |
| 本地化支持 | 多语言（LOCTEXT） | 跟随编辑器语言设置 |

---

**文档版本：v1.4**
**最后更新：2025-11-29**
**变更记录：更新 UI 设计章节，链接到详细 UI 开发文档**
