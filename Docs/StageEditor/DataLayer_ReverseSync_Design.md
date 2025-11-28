# DataLayer 反向同步方案设计文档

> 创建日期: 2025-11-22
> 状态: 待实施 (方案确认: 混合模式 + 智能匹配)

## 1. 需求背景

### 1.1 使用场景

1. **中途接入**: 项目已使用DataLayer管理场景，希望迁移到StageEditor
2. **团队协作**: 美术/关卡设计师用DataLayer Outliner创建结构，程序用StageEditor管理
3. **现有资产复用**: 不想重建DataLayer，希望直接导入

### 1.2 架构理解（关键约束）

**✅ 正确的DataLayer-Stage-Act层级关系：**

```
DataLayer Outliner (必须有层级)      StageEditor (映射关系)
├── DL_Stage_MainStage           →  Stage: MainStage (绑定主DataLayer)
│   ├── DL_Act_MainStage_Default →      ├── Act 0: Default (绑定子DataLayer)
│   ├── DL_Act_MainStage_Day     →      ├── Act 1: Day
│   └── DL_Act_MainStage_Night   →      └── Act 2: Night
└── DL_Stage_CombatStage         →  Stage: CombatStage
    ├── DL_Act_CombatStage_Default →    ├── Act 0: Default
    └── DL_Act_CombatStage_Arena  →     └── Act 1: Arena
```

**映射规则（强制约束）：**
| 元素 | DataLayer绑定 | 层级关系 |
|------|--------------|----------|
| **Stage** | 父DataLayer | 顶层 |
| **Act (普通)** | 子DataLayer | Stage主DataLayer的子层 |
| **Act 0 (Default)** | 子DataLayer | 也必须绑定（特殊但强制） |
| **一一对应** | 每个DataLayer只能映射一个Stage/Act | 违反则报错 |

**为什么必须保留层级？**
- 通过Stage DataLayer的加载/卸载，可以自动管理所有Act DataLayer
- 子DataLayer继承父DataLayer的流送状态
- 符合UE5 DataLayer设计的最佳实践

---

## 2. 命名规范（智能匹配依据）

### 2.1 标准命名格式

```
Stage DataLayer:  DL_Stage_{StageName}
Act DataLayer:    DL_Act_{StageName}_{ActName}
```

**示例：**
```
✅ 正确命名:
DL_Stage_MainStage
DL_Act_MainStage_Default
DL_Act_MainStage_Day
DL_Act_MainStage_Night

❌ 错误命名（无法智能匹配）:
MainStage_DL           → 没有 "DL_Stage_" 前缀
DL_Day                 → 无法识别所属Stage
DL_Act_Day             → 缺少Stage名称部分
```

### 2.2 命名解析规则

```cpp
// 正则匹配规则
Stage Pattern: ^DL_Stage_(.+)$                    → 捕获: StageName
Act Pattern:   ^DL_Act_([^_]+)_(.+)$              → 捕获: StageName, ActName
```

**解析示例：**
| DataLayer名称 | 类型 | StageName | ActName |
|---------------|------|-----------|---------|
| DL_Stage_Combat | Stage | Combat | - |
| DL_Act_Combat_Arena | Act | Combat | Arena |
| DL_Act_Combat_Default | Act | Combat | Default |

### 2.3 用户警告机制

导入时如果检测到不规范命名，显示警告：

```
⚠️ Warning: Non-standard DataLayer names detected

The following DataLayers cannot be auto-matched due to naming:
  • MainStage_DL (missing "DL_Stage_" prefix)
  • DL_Day (cannot determine parent Stage)

Recommendation:
  Rename them to: DL_Stage_MainStage, DL_Act_MainStage_Day

You can still manually link these DataLayers using "Link Existing DataLayer".
```

---

## 3. 方案选项

### 方案A: 手动关联模式

**流程:**
1. 用户在StageEditor创建Stage和Act
2. Act右键菜单添加"Link Existing DataLayer"选项
3. 弹出DataLayer选择器，用户选择要关联的DataLayer
4. 系统将选中的DataLayerAsset关联到Act

**优点:**
- 实现简单
- 用户完全控制映射关系

**缺点:**
- 手动操作繁琐

---

### 方案B: 批量导入向导

**流程:**
1. 提供"Import from DataLayers"向导
2. 扫描所有DataLayerInstance，展示列表
3. 用户选择要导入的DataLayer
4. 系统自动创建Stage/Act并关联

**优点:**
- 批量处理效率高

**缺点:**
- 实现复杂

---

### 方案C: 智能匹配模式

**流程:**
1. 基于命名规范自动解析
2. 自动识别Stage和Act的对应关系
3. 一键导入

**优点:**
- 自动化程度最高

**缺点:**
- 命名不规范时失效

---

### 方案D: 混合模式 + 智能匹配（最终方案 ✅）

结合A+B+C：

| 功能 | 入口 | 用途 |
|------|------|------|
| **Smart Import** | Toolbar按钮 | 智能匹配导入所有规范命名的DataLayer |
| **Link DataLayer** | Act右键 | 手动关联单个DataLayer |
| **Import as Acts** | Stage右键 | 批量导入为指定Stage的Acts |

---

## 4. 核心功能设计

### 4.1 Smart Import（智能导入）

**UI位置**: StageEditor Panel顶部工具栏

```
┌────────────────────────────────────────────────────────┐
│ [Register Props] [Refresh] [Smart Import DataLayers]  │
└────────────────────────────────────────────────────────┘
```

**点击后弹出向导：**

```
┌─────────────────────────────────────────────────────────┐
│ Smart Import DataLayers                                  │
├─────────────────────────────────────────────────────────┤
│ Scanning DataLayers...                                   │
│                                                          │
│ ✅ Matched Stages (2):                                   │
│   ☑ DL_Stage_MainStage → Stage: MainStage               │
│       ☑ DL_Act_MainStage_Default → Act 0: Default       │
│       ☑ DL_Act_MainStage_Day → Act 1: Day               │
│       ☑ DL_Act_MainStage_Night → Act 2: Night           │
│   ☑ DL_Stage_Combat → Stage: Combat                     │
│       ☑ DL_Act_Combat_Default → Act 0: Default          │
│       ☑ DL_Act_Combat_Arena → Act 1: Arena              │
│                                                          │
│ ⚠️ Unmatched DataLayers (3):                             │
│   • MainStage_Legacy (no "DL_Stage_" prefix)            │
│   • DL_DayScene (missing Stage reference)               │
│   • DL_Act_Test (no matching parent Stage)              │
│                                                          │
│ Options:                                                 │
│ ☑ Register actors with StagePropComponent as Props      │
│ ☐ Register all actors as Props                          │
│                                                          │
│ [View Naming Guide]  [Cancel]  [Import Selected]        │
└─────────────────────────────────────────────────────────┘
```

### 4.2 Link Existing DataLayer（手动关联）

**入口**: Act右键菜单

```
┌─────────────────────────────────────┐
│ Link Existing DataLayer              │
├─────────────────────────────────────┤
│ Available DataLayers:                │
│ ○ DL_Act_MainStage_Day               │
│ ○ DL_Act_MainStage_Night             │
│ ● DL_Act_Combat_Arena    ← 选中      │
├─────────────────────────────────────┤
│ ☑ Register actors with               │
│   StagePropComponent as Props        │
├─────────────────────────────────────┤
│ ⚠️ Note: This DataLayer will be      │
│ exclusively linked to this Act       │
│                                      │
│        [Cancel]  [Link]              │
└─────────────────────────────────────┘
```

### 4.3 Import DataLayers as Acts（批量导入到指定Stage）

**入口**: Stage右键菜单

```
┌─────────────────────────────────────────────────┐
│ Import DataLayers to Stage: MainStage           │
├─────────────────────────────────────────────────┤
│ Select Child DataLayers:                        │
│ ☑ DL_Act_MainStage_Day       → Act: Day        │
│ ☑ DL_Act_MainStage_Night     → Act: Night      │
│ ☐ DL_Act_Combat_Arena        (skip)            │
├─────────────────────────────────────────────────┤
│ ☑ Register actors with StagePropComponent       │
├─────────────────────────────────────────────────┤
│              [Cancel]  [Import]                 │
└─────────────────────────────────────────────────┘
```

---

## 5. 核心API设计

```cpp
// StageEditorController.h

//----------------------------------------------------------------
// DataLayer Reverse Sync & Smart Import
//----------------------------------------------------------------

/** Smart import: Parse all DataLayers and create Stages/Acts based on naming */
bool SmartImportDataLayers(const TArray<UDataLayerAsset*>& SelectedDataLayers, bool bRegisterPropsWithComponent, bool bRegisterAllProps);

/** Parse DataLayer name to extract Stage/Act info */
struct FDataLayerParseResult
{
    bool bIsValid;
    bool bIsStageLayer;      // true = Stage, false = Act
    FString StageName;
    FString ActName;         // Only for Act DataLayers
};
FDataLayerParseResult ParseDataLayerName(const FString& DataLayerName) const;

/** Get all DataLayers and group by hierarchy */
struct FDataLayerHierarchy
{
    UDataLayerAsset* StageDataLayer;
    TArray<UDataLayerAsset*> ActDataLayers;  // Children
};
TArray<FDataLayerHierarchy> AnalyzeDataLayerHierarchy() const;

/** Links an existing DataLayer to an Act (checks for conflicts) */
bool LinkDataLayerToAct(int32 ActID, UDataLayerAsset* ExistingDataLayer, bool bRegisterActors);

/** Links an existing DataLayer to a Stage */
bool LinkDataLayerToStage(AStage* Stage, UDataLayerAsset* ExistingDataLayer);

/** Imports child DataLayers as Acts for a specific Stage */
bool ImportDataLayersAsActs(AStage* TargetStage, const TArray<UDataLayerAsset*>& DataLayers, bool bRegisterActors);

/** Gets all actors in a DataLayer that have StagePropComponent */
TArray<AActor*> GetPropsInDataLayer(UDataLayerAsset* DataLayer) const;

/** Gets all actors in a DataLayer (regardless of component) */
TArray<AActor*> GetAllActorsInDataLayer(UDataLayerAsset* DataLayer) const;

/** Checks if a DataLayer is already linked to any Stage/Act */
bool IsDataLayerLinked(UDataLayerAsset* DataLayer) const;
```

---

## 6. 工作流程

### 6.1 Smart Import流程

```
[Smart Import按钮]
    ↓
AnalyzeDataLayerHierarchy()
    ↓
For each DataLayer:
    ParseDataLayerName() → 识别Stage/Act
    ↓
显示向导（已匹配 + 未匹配）
    ↓
用户选择要导入的项
    ↓
For each matched Stage:
    ├── 创建Stage (如不存在)
    ├── LinkDataLayerToStage()
    └── For each Act DataLayer:
        ├── CreateNewAct()
        ├── LinkDataLayerToAct()
        └── (可选) 注册Prop
```

### 6.2 Default Act的DataLayer绑定

```
When creating Stage:
    ├── CreateDataLayerForStage() → 创建Stage主DataLayer
    └── CreateDataLayerForAct(ActID=0) → 创建Default Act的子DataLayer
            └── 命名: DL_Act_{StageName}_Default
```

---

## 7. 实施计划

### Phase 1: 基础解析与查询 ✅
- [x] `ParseDataLayerName()` - 命名规则解析 (支持DL_Stage_和DL_Act_前缀)
- [x] `AnalyzeDataLayerHierarchy()` - 层级分析 (分组Stage和Act)
- [x] `IsDataLayerLinked()` - 冲突检测 (检查是否已被Act关联)
- [x] `GetPropsInDataLayer()` / `GetAllActorsInDataLayer()` - Actor查询

### Phase 2: 手动关联功能 ✅
- [x] `LinkDataLayerToAct()` - 关联DataLayer到Act（带冲突检测和Actor注册）
- [x] `LinkDataLayerToStage()` - 关联DataLayer到Stage
- [x] Act右键菜单："Link Existing DataLayer"
- [x] 简单选择对话框（显示第一个可用DataLayer）

### Phase 3: 批量导入
- [ ] `ImportDataLayersAsActs()` - 导入子DataLayer为Acts
- [ ] Stage右键菜单："Import DataLayers as Acts"

### Phase 4: Smart Import核心
- [ ] Smart Import按钮
- [ ] 匹配/未匹配分组显示
- [ ] 命名规范警告
- [ ] Actor同步选项

### Phase 5: Default Act DataLayer
- [ ] 修改`CreateNewAct()` - Act 0自动创建子DataLayer
- [ ] 命名规则: `DL_Act_{StageName}_Default`

### Phase 6: 优化与完善
- [ ] 命名规范文档 (UI内嵌)
- [ ] Undo/Redo支持
- [ ] 导入预览

---

## 8. 已确认的设计决策

| 问题 | 决策 | 原因 |
|------|------|------|
| **Q1: DataLayer层级** | ✅ B - 必须保留层级 | 父DataLayer控制子DataLayer流送 |
| **Q2: Act命名** | ✅ B - 去掉前缀 | `DL_Act_MainStage_Day` → Act: "Day" |
| **Q3: 重复关联** | ✅ C - 报错阻止 | 一一对应，严格映射 |
| **Q4: Actor同步** | ✅ C - 用户选择 | 默认：只同步带`UStagePropComponent`的Actor |

---

## 9. 命名规范速查卡（提供给用户）

```
╔═══════════════════════════════════════════════════════╗
║  StageEditor DataLayer Naming Convention              ║
╠═══════════════════════════════════════════════════════╣
║                                                       ║
║  Stage DataLayer:                                     ║
║    Format: DL_Stage_{StageName}                       ║
║    Example: DL_Stage_MainStage                        ║
║                                                       ║
║  Act DataLayer (must be child of Stage DataLayer):   ║
║    Format: DL_Act_{StageName}_{ActName}               ║
║    Example: DL_Act_MainStage_Day                      ║
║                                                       ║
║  Default Act DataLayer:                               ║
║    Format: DL_Act_{StageName}_Default                 ║
║    Example: DL_Act_MainStage_Default                  ║
║                                                       ║
║  ⚠️ Important:                                         ║
║    • Act DataLayers must be children of Stage DL      ║
║    • Non-standard names require manual linking        ║
║                                                       ║
╚═══════════════════════════════════════════════════════╝
```

---

**方案已确认，准备实施。**
