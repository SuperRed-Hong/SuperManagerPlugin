# StageEditor 开发上下文总结

> 日期: 2025-11-22
> 分支: `StageEditor-Dev-Phase-1`
> 状态: DataLayer反向同步Phase 1-2已完成，多人协作改进待实施

---

## 当前已完成的工作

### 1. DataLayer正向集成（已完成 ✅）

**实现文件：**
- `StageEditorController.h:89-94` - API声明
- `StageEditorController.cpp:675-872` - 实现

**核心功能：**
```cpp
// 创建DataLayerAsset并保存到Content
UDataLayerAsset* CreateDataLayerAsset(const FString& AssetName, const FString& FolderPath);

// 获取或创建DataLayerInstance
UDataLayerInstance* GetOrCreateInstanceForAsset(UDataLayerAsset* Asset);

// 为Stage创建DataLayer（Asset + Instance）
bool CreateDataLayerForStage(AStage* Stage);

// 为Act创建DataLayer（Asset + Instance），作为Stage子层
bool CreateDataLayerForAct(int32 ActID);
```

**资产目录结构：**
```
/Game/StageEditor/DataLayers/
├── DL_Stage_{StageName}.uasset
├── DL_Act_{StageName}_{ActID}_{ActName}.uasset
└── ...
```

---

### 2. DataLayer反向同步 Phase 1-2（已完成 ✅）

**设计文档：** `Docs/StageEditor/DataLayer_ReverseSync_Design.md`

#### Phase 1: 基础解析与查询
```cpp
// StageEditorController.h:126-157

// 解析DataLayer命名（DL_Stage_XXX / DL_Act_XXX_XXX）
FDataLayerParseResult ParseDataLayerName(const FString& DataLayerName) const;

// 检测DataLayer是否已被关联（冲突检测）
bool IsDataLayerLinked(UDataLayerAsset* DataLayer) const;

// 获取DataLayer中的Actor
TArray<AActor*> GetPropsInDataLayer(UDataLayerAsset* DataLayer) const;
TArray<AActor*> GetAllActorsInDataLayer(UDataLayerAsset* DataLayer) const;

// 分析DataLayer层级结构
TArray<FDataLayerHierarchy> AnalyzeDataLayerHierarchy() const;
```

#### Phase 2: 手动关联功能
```cpp
// StageEditorController.h:159-163

// 关联现有DataLayer到Act（带冲突检测）
bool LinkDataLayerToAct(int32 ActID, UDataLayerAsset* ExistingDataLayer, bool bRegisterActors);

// 关联现有DataLayer到Stage
bool LinkDataLayerToStage(AStage* Stage, UDataLayerAsset* ExistingDataLayer);
```

**UI入口：**
- Act右键菜单 → "Link Existing DataLayer" (StageEditorPanel.cpp:992)
- 显示简单选择对话框（StageEditorPanel.cpp:1699）

---

### 3. 其他改进

#### 地图变化监听（已完成 ✅）
- **问题：** WorldPartition转换后，StageEditorPanel不刷新
- **解决：** 监听`FEditorDelegates::OnMapOpened`，检测WP状态变化时重建UI
- **文件：** StageEditorPanel.h:222-235, StageEditorPanel.cpp:239-270

#### 选择同步改进（已完成 ✅）
- **问题：** 右键Stage行时，同步选择导致视口选中项被改为Stage，"Register Selected Actors"功能失效
- **解决：** 只在`OnMouseClick`时同步视口，右键不同步
- **文件：** StageEditorPanel.cpp:719-720

---

## 当前待实施的工作

### 🔥 优先级1: 多人协作架构改进

**设计文档：** `Docs/StageEditor/MultiUser_Collaboration_Fixes.md`

**问题：**
1. `StageDataLayerName`使用字符串查找，多人创建同名DataLayer会冲突
2. UI使用`ActorLabel`作为显示名称，重名时无法区分

**解决方案：**
- Stage添加`StageDataLayerAsset`字段（Asset引用代替字符串）
- Controller所有查找逻辑改用Asset指针比较
- UI重名检测，添加GUID后缀显示

**实施步骤：** 见 `MultiUser_Collaboration_Fixes.md` 的Phase 1-6

---

### 优先级2: DataLayer反向同步 Phase 3-4（Smart Import）

**设计文档：** `Docs/StageEditor/DataLayer_ReverseSync_Design.md`

**待实施：**
- Phase 3: 批量导入（Stage右键 → Import DataLayers as Acts）
- Phase 4: Smart Import按钮（智能匹配所有规范命名的DataLayer）

**命名规范：**
```
Stage DataLayer:  DL_Stage_{StageName}
Act DataLayer:    DL_Act_{StageName}_{ActName}
Default Act:      DL_Act_{StageName}_Default
```

---

## 项目结构概览

### 核心文件

| 文件 | 职责 |
|------|------|
| `Stage.h/cpp` | 数据模型（Runtime） |
| `StageEditorController.h/cpp` | 业务逻辑（Controller） |
| `StageEditorPanel.h/cpp` | UI（View） |

### 文档目录
```
Docs/StageEditor/
├── HighLevelDesign.md                        # 总体架构设计
├── StageEditorController.md                  # Controller详细设计
├── DataLayer_Integration_Design.md           # DataLayer正向集成 ✅
├── DataLayer_ReverseSync_Design.md           # 反向同步方案（Phase 1-2完成）
├── MultiUser_Collaboration_Fixes.md          # 多人协作改进方案 🔥
└── Development_Context_2025-11-22.md         # 本文档
```

---

## 编译与测试

### 编译命令
```bat
"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat" ^
  ExtendEditorEditor Win64 Development ^
  "-Project=D:/UEProject/ReserchPJ/ExtendEditor/ExtendEditor/ExtendEditor.uproject" ^
  -WaitMutex
```

### 当前编译状态
✅ 所有代码编译通过（截至2025-11-22）

### 测试场景
1. ✅ 创建Stage → 自动创建Stage DataLayer
2. ✅ 创建Act → 创建Act DataLayer（子层）
3. ✅ Act右键 → Link Existing DataLayer → 关联现有DataLayer
4. ⚠️ 多人协作场景（待测试）

---

## 架构关键约束

### DataLayer层级关系（强制）
```
DataLayer Outliner                StageEditor
├── DL_Stage_MainStage        →  Stage: MainStage (主DataLayer)
│   ├── DL_Act_MainStage_Default → Act 0: Default
│   ├── DL_Act_MainStage_Day  →   Act 1: Day
│   └── DL_Act_MainStage_Night→   Act 2: Night
```

- **Stage** → 父DataLayer（顶层）
- **Act** → 子DataLayer（必须是Stage DataLayer的子层）
- **Default Act** → 也必须绑定子DataLayer
- **一一对应** → 每个DataLayer只能映射一个Stage/Act

### MVC模式严格遵守
- **Model**: `AStage` (Runtime数据)
- **View**: `SStageEditorPanel` (Slate UI)
- **Controller**: `FStageEditorController` (唯一通信桥梁)
- View **不得**直接修改Model

---

## Git状态

### 当前分支
```
StageEditor-Dev-Phase-1
```

### 未提交的修改
```
M  Plugins/StageEditor/Source/StageEditor/Private/EditorLogic/StageEditorController.cpp
M  Plugins/StageEditor/Source/StageEditor/Private/EditorUI/StageEditorPanel.cpp
M  Plugins/StageEditor/Source/StageEditor/Public/EditorLogic/StageEditorController.h
M  Plugins/StageEditor/Source/StageEditor/Public/EditorUI/StageEditorPanel.h
?? Docs/StageEditor/DataLayer_Integration_Design.md
?? Docs/StageEditor/DataLayer_ReverseSync_Design.md
?? Docs/StageEditor/MultiUser_Collaboration_Fixes.md
?? Docs/StageEditor/Development_Context_2025-11-22.md
```

### 建议提交策略
```bash
# 先提交DataLayer反向同步Phase 1-2
git add Plugins/StageEditor/
git add Docs/StageEditor/DataLayer_ReverseSync_Design.md
git commit -m "feat: DataLayer reverse sync Phase 1-2 (parse, link, query)"

# 再提交文档
git add Docs/StageEditor/MultiUser_Collaboration_Fixes.md
git add Docs/StageEditor/Development_Context_2025-11-22.md
git commit -m "docs: Add multi-user collaboration fixes and context summary"
```

---

## 下一步行动建议

### 立即执行（新对话开始）
1. 阅读 `MultiUser_Collaboration_Fixes.md`
2. 实施 **Phase 1: Stage DataLayer Asset引用**
3. 测试编译

### 后续计划
1. Phase 2-6: 完成多人协作改进
2. DataLayer反向同步 Phase 3-4（Smart Import）
3. 完整多人协作场景测试

---

## 关键上下文信息

### 用户偏好
- 实施前需要提供 **Implementation Plan** 文档供审阅
- 文档使用中文 + 英文代码注释
- 重视多人协作和命名规范

### 技术栈
- UE 5.6
- World Partition + DataLayer
- MVC架构
- Slate UI

### 开发环境
- Windows
- Visual Studio 2022
- Git版本控制

---

**准备好接力开发！建议从多人协作改进Phase 1开始。**
