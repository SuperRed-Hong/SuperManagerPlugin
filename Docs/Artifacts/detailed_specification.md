# 详细功能规格说明书 (Detailed Specification)

> [!IMPORTANT]
> **📋 唯一真实数据源 (Single Source of Truth)**
> 
> 本文档是 Stage Editor 插件的**唯一技术规格文档**。所有设计决策、架构方案、开发计划均以此文档为准。
> 
> 其他文档（`implementation_plan.md`, `design_review_and_roadmap.md`）已过时，仅作历史参考。

> [!NOTE]
> **开发策略**: 采用三阶段迭代开发
> - **Phase 1 (原型)**: 核心 Stage-Act-Prop 循环，本地 ID，基础 UI
> - **Phase 2 (功能填充)**: DataLayer 集成，服务器 ID，Editor Mode 交互
> - **Phase 3 (打磨)**: UX 优化，错误处理，性能调优

## 1. ID 系统与数据结构 (ID System & Data Structures)

### 1.1 核心数据结构
我们采用层级化的 ID 结构来确保数据的唯一性和查询效率。

*   **`FStageHierarchicalId`**:
    *   `int32 StageID`: 全局唯一，代表一个 `AStage` 实例。
    *   `int32 ActID`: 局部唯一，代表 Stage 内的一个状态（幕）。
    *   `int32 PropID`: 局部唯一，代表 Stage 内的一个受控对象。

### 1.2 ID 分配与关联策略 (ID Allocation & Linking Strategy) [已定稿]

**核心决策**: 采用 **"引用关联 (Reference-Based)"** 模式，而非直接存储数值 ID。

*   **数据结构设计**:
    *   **`AStage`**: 存储权威的 `int32 StageID` (由服务器分配或离线临时生成)。
    *   **`UStagePropComponent`**: 仅存储指向 `AStage` 的软引用 (`TSoftObjectPtr<AStage> OwnerStage`) 和自身的局部 `int32 PropID`。**绝不存储** `StageID` 数值。
    *   **运行时解析**: Prop 需要完整 ID 时，通过 `OwnerStage.Get()->StageID` 动态获取。

*   **离线工作流 (Offline Workflow)**:
    1.  **创建**: 用户离线创建 `AStage`，系统分配临时负数 ID (如 `-1`)。
    2.  **关联**: 用户在该 Stage 下创建大量 Prop。Props 记录 `OwnerStage` 指向该 Stage Actor。
    3.  **修复 (Fix-up)**: 
        *   用户联网，点击"修复 ID"。
        *   系统向服务器申请正式 ID (如 `1001`)。
        *   系统**仅修改并保存 `AStage` 这一份文件** (`StageID: -1 -> 1001`)。
    4.  **结果**: 所有 Prop 自动归属于 ID `1001` 的 Stage。

> [!IMPORTANT]
> **决策理由 (Rationale)**: 
> 如果 Prop 直接存储 `StageID`，在 ID 变更（如离线转在线）时，需要遍历并修改所有归属的 Prop 文件。在 **One File Per Actor (OFPA)** 机制下，这会导致成千上万个文件被标记为 Dirty，造成严重的 IO 压力和版本控制混乱。
> **引用关联方案** 完美规避了此问题，将修改范围限制在单个 `AStage` 文件内。

> [!NOTE]
> **原型阶段策略 (Prototype Phase Strategy)**:
> 在当前的 Phase 1 (原型) 开发中，我们将**跳过**服务器交互。`StageID` 将由本地简单的计数器或随机数生成器发放，优先保证核心流程（Stage-Act-Prop）跑通。服务器对接将在 Phase 2 进行。

### 1.3 核心概念 (Core Concepts)
*   **Stage (舞台)**: `AStage` Actor。区域管理器，负责加载/卸载 DataLayer，并向 Prop 发送状态指令。
*   **Act (幕)**: `FAct` 结构体。定义了某一时刻下，一组 Prop 应该处于什么状态。
*   **Prop (道具)**: **混合模式 (Inheritance + Composition)**
    *   **`UStagePropComponent`**: 核心组件，封装所有 Prop 逻辑。
        *   存储 `PropID`, `PropState`, `OwnerStage` (软引用)。
        *   提供 `OnPropStateChanged` 委托。
        *   任何 Actor 都可以手动挂载此组件成为 Prop。
    *   **`AProp`**: 便利基类 (Optional)。
        *   自动包含 `UStagePropComponent`。
        *   为简化使用而存在（用户直接继承 `AProp` 即可，无需手动添加 Component）。
        *   所有核心逻辑仍在 Component 中，`AProp` 只是一个薄封装。
    *   **设计优势**: 
        *   灵活性：任何现有 Actor 可通过添加 Component 变成 Prop。
        *   便利性：新 Actor 可直接继承 `AProp` 快速开始。

## 2. 编辑器模式交互 (Editor Mode UX) [部分定稿]

### 2.1 入口与激活 (Entry Points) [已定稿]
为了提供流畅的体验，我们将提供三种打开 Stage 编辑面板的方式：
1.  **上下文菜单 (Context Menu)**: 在视口中右键点击 `AStage` Actor -> 菜单中显示 "Open Stage Editor" 按钮 (通过 `LevelEditorMenuExtender` 实现)。
2.  **模式自动激活 (Auto-Open)**: 切换到 "Stage Editor Mode" 时，自动在左侧功能栏下方打开并停靠面板。
3.  **模式工具栏 (Mode Toolbar)**: "Stage Editor Mode" 的左侧工具栏中包含一个显式的 "Open Panel" 按钮。

### 2.2 面板布局与层级结构 (Panel Layout & Hierarchy) [已定稿]
**目标**: 实现类似 `SceneOutliner` 的层级视图，直观展示 Stage-Act-Prop 关系。
**核心逻辑**: `AStage` 构造时自动包含一个 **"Default Act"**。所有新注册的 Prop 默认自动加入 Default Act。

**提案设计**: 采用 **"以 Act 为中心 (Act-Centric)"** 的树状视图。

*   **根节点**: 当前编辑的 `AStage` 名称。
    *   **文件夹: Acts (幕)**
        *   **节点: Default Act** (默认幕 - 包含所有 Prop)
            *   *子节点*: **Prop 1** (State: Default)
            *   *子节点*: **Prop 2** (State: Default)
            *   *... (所有注册的 Prop 都在这里)*
        *   **节点: Act A** (自定义幕)
            *   *子节点*: **Prop 1** (State: Open)
            *   *(仅显示在此 Act 中有特定状态覆写的 Prop)*
    *   **文件夹: Registered Props (道具池)**
        *   *(用于查看 Prop 被哪些 Act 引用，或进行批量管理)*

### 2.3 注册逻辑 (Registration Logic) [已定稿]
我们将同时实现两种注册方式，且注册后 **自动加入 Default Act**：
1.  **拖拽 (Drag & Drop)**: 选中视口/大纲中的 Actor -> 拖拽到 Stage 面板。
    *   *逻辑*: 检测 Actor 是否包含 `UStagePropComponent` -> 注册到 Stage -> 自动添加到 Default Act -> 默认状态为 0。
2.  **吸管/添加 (Pipette/Add)**: 点击面板上的 "Add Prop" 按钮 -> 拾取 Actor。
    *   *逻辑*: 同上。

### 2.4 视口交互与可视化 (Viewport & Visualization) [已定稿]
*   **选择同步**: 点击视口 Prop -> 面板高亮对应条目。
*   **连线可视化**: 选中 `AStage` 或特定 `Act` 时，绘制虚线连接所有相关的 Prop（通过 Component 查找），直观展示控制范围。

## 3. DataLayer 集成 (DataLayer Integration)

### 3.1 职责划分
*   `AStage` 持有对 `UDataLayerAsset` 的引用。
*   `FAct` 绑定具体的 `UDataLayerInstance` (或者 Asset，取决于 UE5 版本)。

### 3.2 编辑器内预览 (Editor Preview) [分阶段实现]
*   **Phase 1 (PropState)**:
    *   实现 "👁️ (Preview)" 按钮。
    *   **功能**: 仅强制刷新场景中 Prop 的 `PropState`，让它们表现出"开门/关门"等状态。
    *   **实现难度**: 低。直接调用 Actor 的 `ConstructionScript` 或自定义事件即可。
*   **Phase 2 (DataLayer)**:
    *   **功能**: 增加对 DataLayer 可见性的控制。
    *   **实现难度**: 中。依赖于 DataLayer 系统的完整集成。
    *   **结论**: 原型阶段先做 PropState 预览，DataLayer 预览留到 Phase 2。

### 3.3 Stage 与 DataLayer 的加载关系 (Stage Loading Strategy) [已定稿]
根据 `HighLevelDesign.md` 的设计原则，我们需要明确 `AStage` 自身与 DataLayer 的关系：

1.  **AStage Actor (常驻指挥官)**:
    *   **加载策略**: `AStage` Actor 自身应设置为 **"Always Loaded" (非 Spatially Loaded)**。
    *   **理由**: 它是逻辑控制器，必须常驻内存以便 `StageManagerSubsystem` 随时通过 ID 找到它并下达指令（如任务系统远程激活某区域）。它的内存占用极小。
2.  **Stage Content (流送内容)**:
    *   **加载策略**: 受控的 Prop Actor 和场景静态网格体必须分配给具体的 **DataLayer**。
    *   **理由**: 这些是重资产。它们默认是卸载的，只有当 `AStage` 激活了特定的 `Act` 时，才会请求 DataLayer 系统加载这些内容。
3.  **管理层级**:
    *   **World Partition** -> 加载 -> **AStage (Lightweight)**
    *   **AStage** -> 激活 Act -> 加载 -> **DataLayer (Heavy Assets)**

### 3.4 DataLayer 层级结构 (DataLayer Hierarchy) [已定稿]
采用以下层级结构：

*   **结构**:
    *   **Parent DataLayer (对应 Stage)**: 例如 `DL_Stage_AncientRuins`。
    *   **Child DataLayer (对应 Act)**: 例如 `DL_Act_Intact`, `DL_Act_Destroyed`。
*   **行为逻辑**:
    *   **组织性**: 在 DataLayer Outliner 中，所有相关的 Act 都会自动折叠在 Stage 父项下，整洁有序。
    *   **级联控制**: 运行时，`AStage` 默认逻辑是：激活 Act -> 自动确保 Parent DL 也被加载。
    *   **例外处理 (Standalone Mode) [⚠️ 待验证 - 关键风险点]**: 
        *   **目标**: 支持只激活 Act (Child DL) 而不激活 Stage (Parent DL)。
        *   **不确定性**: UE5 的 DataLayer 激活逻辑是否强制要求父层级必须加载？虽然理论上 Instance 是独立的，但我们需要实测验证。
        *   **行动项**: 在 Phase 2 开发 DataLayer 功能时，**必须优先编写测试用例**验证这一点。如果 UE 强制级联，我们需要寻找替代方案（如平行 DataLayer 结构）。
    *   **注意**: `AStage` Actor 本身**不应**放入 Parent DL 中，它必须保持在主分区 (Always Loaded) 以便执行逻辑。只有"舞台上的布景 (Props/Meshes)"才放入 DL。

## 4. PropState 扩展性 (Extensibility)

### 4.1 [已定稿] 数据类型
*   目前设计仅支持 `int32`。
*   **用户反馈**: "复杂状态是由 Prop 自己的蓝图内部逻辑自己决定的，Act 只是提供一种预设以及 DataLayer 的标识"。
*   **结论**: 保持 `int32` 作为状态标识符。Prop 内部逻辑（蓝图）负责解析这个 ID 并执行复杂的表现逻辑（如播放 Timeline、切换材质等）。

---

### 总结：待确认决策点
1.  **ID 离线方案**: 是否同意使用"负数 ID + 后期修复"的方案？
2.  **注册交互**: 是否同意主要使用"大纲视图拖拽"的方式？ -> **[用户暂时同意]**
3.  **编辑器预览**: 是否需要实现"不运行游戏时的 Act 预览"？ -> **[已确认 Phase 1 实现 PropState 预览]**
