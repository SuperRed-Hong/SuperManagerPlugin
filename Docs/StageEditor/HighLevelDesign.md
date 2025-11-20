# “关卡编辑器”系统概要设计文档 (High-Level Design)_V4.1

---

### **“动态舞台”系统概要设计文档 (High-Level Design)**

**文档版本:** 4.1 **(最终版 - 全面修订)**

**关联文档:** 《“动态舞台”系统需求说明书 (PRD)》 V2.1

**核心设计原则:** **区域化关卡蓝图**，**MVC关卡编辑器模块， 多人协作友好**
文档更新：

| 文档版本 | 作者 | 更新内容 | 更新日期 |
| --- | --- | --- | --- |
| V4.0 | 洪硕 | Act优先级的设计不再通过Priority参数，采用最新优先的设计. v4 | 9.20 |
| v4.1 | 洪硕 | 细化【通用】ID系统概要设计 | 9.22 |

---

### **1. 引言**

### **1.1 范围**

本设计覆盖了“动态舞台”系统的两大核心组成部分：**【运行时核心架构】** 和 **【编辑器工具链】**。它将明确划分系统模块、定义各模块的核心职责与交互接口。

**1.2 执行摘要：**

- **`AStage` Actor :** 策划在关卡中放置`AStage` Actor来定义一个逻辑区域。这个Actor是**常驻内存、自我驱动的**。它通过监听玩家与触发区域（`TriggerZone`）的交互事件，来**自动管理**其负责区域内所有资源的**加载/卸载**（通过`Data Layer`）。
- **`Act` & `PropState` (数据驱动的状态):** `AStage`的蓝图逻辑根据游戏进程（如任务），来决定激活哪个**场景状态参数 (`Act`)**。`Act`的核心是一份数据表，它为区域内的**受控道具 (`AProp`)** 指定一个目标**状态ID (`PropState`)**。
- **`AProp` Actor (状态的最终表现):** `AStage`将计算出的最终`PropState`值**推送**给对应的`AProp`实例。`AProp`自身的蓝图则负责**解释**这个整数ID，并执行最终的视觉与逻辑表现（如改变模型、播放特效）。

---

### **2. 系统架构总览**

### **2.1 设计模式**

本系统将严格遵循**模型-视图-控制器** (MVC)的设计模式，以确保**高内聚低耦合**。主要应用于**【编辑器工具链】**与**【运行时核心架构】**的交互方式上：

- **模型 (Model):** 整个**【运行时核心架构】**，包括`AStage` Actor、`AProp` Actor、`FAct`等所有数据结构。这是系统的**“唯一真实数据源 (Single Source of Truth)”**。策划在编辑器中所做的所有配置，最终都将序列化并保存在`AStage` Actor中。
- **视图 (View):** 编辑器中的所有Slate UI界面，特别是`SStageEditorPanel`。视图的职责**只负责呈现**模型（`AStage`）的数据，并将用户的输入操作（点击、拖拽等）通知给控制器。
- **控制器 (Controller):** 一个专门的`FStageEditorController` C++类。它是**连接视图 (View)**与**模型** **(Model)**的唯一桥梁。它负责**接收视图的通知**，**执行所有业务逻辑**（如创建`Act`、修改`PropState`），并最终**修改模型的数据**。

### **2.2 系统架构图**

暂时无法在飞书文档外展示此内容

---

### **3. 核心模块职责说明**

### **3.1 【运行时】模块**

- **`StageManagerSubsystem`:**
    - **职责:** 辅助角色。提供一个运行时注册表，供系统快速查找“在线”的`AStage`实例；并提供指令式蓝图API（状态锁），用于游戏逻辑强制覆写`AStage`的默认行为。

### **3.2 【通用】核心数据结构**

- **`AStage` Actor:**
    - **职责:** 自治的区域导演。在`BeginPlay`中绑定其`TriggerZone`的`Overlap`事件，并根据这些事件驱动内部状态机。当状态机进入激活状态时，其蓝图逻辑负责编排`Act`，Act的优先级的设计不再通过Priority参数，采用最新优先的设计应用最终的`PropState`值，并将其**推送**给对应的`AProp`实例。同时，它直接调用`UDataLayerManager`来加载/卸载其主Data Layer。
- **`AProp` Actor:**
    - **职责:** 所有受控单元的**基类**。它包含核心的`int32 PropState`变量，并提供了响应此变量变化的蓝图事件（例如`OnPropStateChanged`）。所有具体的道具（门、NPC、灯）都必须继承自它。
- **`FAct` Struct:**
    - **职责:** “场景状态参数”。一个纯数据结构、关联的Data Layer，以及一个`TMap<FPropID, int32>`，用于存储该`Act`下每个`Prop`的目标`PropState`值。

### 3.3 【通用】ID与索引系统 (概要)

为确保`Stage`, `Act`, `Prop`之间数据关联的绝对稳定和高效查询，系统将采用一套分层的、全局唯一的ID系统。该系统需满足全局唯一性、引用稳定性和高效索引的目标。

- **复合ID结构体:** 我们将定义一个`FStageHierarchicalId`的C++结构体，用于清晰地存储`StageID`, `ActID`, `PropID`三个部分。
- **中心化`StageID`分配:** 全局唯一的`StageID`将通过一个中心化的Web API服务进行权威性分配，以根除多人协作时的冲突。
- **本地化`Prop`索引:** 每个`AStage` Actor将内部维护一个`TMap`形式的“道具注册表”，通过`FStageHierarchicalId`来索引到具体的`AProp` Actor的稳定引用（`TSoftObjectPtr`）。

**补充说明：**

ID系统的核心需求之一，是通过一个中心化的Web服务来分配全局唯一的`StageID`，以根除多人协作时的冲突。

在技术选型阶段，我们意识到这个看似单一的需求，实际上横跨了三个完全不同的技术领域：

1. **后端服务:** 负责实现Web API和数据库，进行权威性的ID生成。
2. **UE客户端网络层:** 负责在UE编辑器中，通过HTTP协议与后端服务进行通信。
3. **UE客户端数据层:** 负责在客户端内部，定义ID的数据结构（`FStageHierarchicalId`）并实现本地的索引机制（`TMap`注册表）。

**核心挑战:**
让任何一个开发者独立负责全部三个领域，都存在巨大的技术风险，且不利于并行开发。

**架构决策：“接口驱动的解耦 (Interface-Driven Decoupling)”**

我们将整个“**ID与索引系统**”的详细设计与开发工作，**主动拆分为三个独立的、职责清晰的模块**。这三个模块之间，仅通过一份预先定义好的、轻量级的“**API接口契约**”进行连接，作为它们之间唯一的“防火墙”。

- **模块一：后端服务 (The Server):**
    - **职责:** 负责“生产”ID。
    - **交付物:** 一个实现了API契约的Web服务。
- **模块二：客户端网络层 (The Messenger):**
    - **职责:** 负责“传递”ID。
    - **交付物:** 一个封装了HTTP通信的C++客户端类 (`FIDManagerClient`)。
- **模块三：客户端数据层 (The Consumer):**
    - **职责:** 负责“使用”ID。
    - **交付物:** 核心ID数据结构 (`FStageHierarchicalId`)和本地索引机制的实现。

**结论:**
因此，在后续的“**详细设计规划**”**中，你会看到这三个模块被作为**独立的任务**进行分派。这不是一次偶然的拆分，而是我们为了保证系统设计的专业性、健壮性以及团队开发效率而做出的**主动架构选择。

- **关于本ID系统的具体位段划分、结构体定义、分配流程和索引机制的详细实现，请参阅独立的[中心化ID分配服务后端详细设计](https://o082e0vcfd2.feishu.cn/wiki/UJZhwHeyKiZvP7k8U8TcQSjGnec)[《FIDManagerClient详细设计文档》](https://o082e0vcfd2.feishu.cn/wiki/I4NOw2UjqiD8WGk5eALcOGf7nkd)[ID核心数据结构与本地索引机制详细设计文档](https://o082e0vcfd2.feishu.cn/wiki/M4oNwttwliIKAvkbbHZc3prRndp)。**

### **3.4 【编辑器】模块**

- **`SStageEditorPanel` (View - 主配置面板):**
    - **职责:** **`Act`和`Prop`的可视化配置界面**。以树状图的形式展示`AStage`的`Act`列表和`Prop`注册表。它负责处理所有与**场景状态**相关的配置（如设置`PropState`，创建`Act`，注册`Prop`等）。
    - **交互:** 它的所有用户操作都将转发给`FStageEditorController`。
- **`FStageEditorController` (Controller - 逻辑中枢):**
    - **职责:** `SStageEditorPanel`的逻辑中枢。封装所有对`AStage`**数据**的修改逻辑（`CreateAct`, `RegisterProp`, `SetPropStateForAct`等），并管理Undo/Redo事务。
    - **交互:** 它是连接`SStageEditorPanel`（View）和`AStage`（Model）的唯一桥梁。
- **`FAGroupDetailCustomization` (View - Details面板自定义):**
    - **职责:** 专门负责`AStage` Actor自身属性**在Details面板中的**UI表现。它的核心任务就是实现“**无感ID申请**”的交互流程。
    - **交互:** 它将接管`StageID`和`GroupName`等属性的默认显示。当用户输入`GroupName`时，它将调用`FIDManagerClient`来发起网络请求，并根据结果更新自身的UI状态（显示加载图标、错误信息等）。
- **`FIDManagerClient` (Service Layer - 网络服务层):**
    - **职责:** **一个独立的、专门负责网络通信的“信使”**。它封装了所有与“中心化ID分配服务”的HTTP交互。
    - **交互:** 它是我们UE客户端与后端API之间的**唯一接口**。`FAGroupDetailCustomization`将是它的主要调用者。它自身不包含任何UI或业务逻辑。

---

### **4. 关键数据流**

### **4.1 编辑器数据流：策划设置PropState**

1. **视图 (View):** 策划在`SStageEditorPanel`的UI上，为一个`Prop`在某个`Act`下的`PropState`输入框中输入了新的值。
2. **控制器 (Controller):** UI将此操作（例如`OnStateValueCommitted`）通知`FStageEditorController`，并传递必要的参数（`ActID`, `PropID`, `NewStateValue`）。
3. **模型 (Model):** `Controller`开启一个`FScopedTransaction`。然后，获取当前编辑的`AStage` Actor的引用，并调用其内部函数来修改`Acts`数组中对应的数据。
4. **视图更新 (View Update):** `Controller`在操作成功后，广播一个委托（如`OnDataModelChanged`）。`SStageEditorPanel`监听到此委托，并刷新其UI以反映最新的数据。

### **4.2 运行时数据流：玩家进入区域并激活Stage**

1. **事件触发 (Event):** 玩家Pawn进入一个与`AStage`绑定的`ATriggerVolume` (`LifecycleZone` / `ActivationZone`)。
2. **Stage响应 (Model Response):** `AVolume`的`Overlap`事件触发，并调用`AStage`上绑定的回调函数，驱动其内部状态机进入`Active`状态。
3. **Act编排 (Orchestration):** `AStage`的蓝图逻辑开始执行。它根据当前游戏状态（如任务进度），决定一个**当前需要激活的`Act`的集合**（例如，同时激活`Act_Default_Layout`和`Act_Quest_A`）。
4. **状态计算 (State Calculation):** `AStage`遍历所有激活的`Act`。根据`Act`的优先级，计算出每一个被影响的`Prop`的**最终`PropState`值**。例如，`Act_Quest_A`的设置会覆盖`Act_Default_Layout`的设置。
5. **状态应用 (State Application):** `AStage`获取到`Prop_A`的最终`PropState`值应为`5`。它找到`Prop_A`的实例，并**直接修改**其`PropState`变量为`5`。
6. **Prop响应 (Prop Reaction):** `Prop_A`的`PropState`变量发生变化，触发其内部的`OnPropStateChanged`事件。`Prop_A`的蓝图根据新值`5`，执行其自定义的行为（例如，播放特效、改变模型）。