# H-006 TriggerZone 系统测试指南

> 本文档指导如何在编辑器中测试 H-006 实现的功能。

---

## 测试准备

### 1. 打开测试地图

```
Content/Maps/StageEditorTestMap_WP
```

### 2. 确认编译成功

运行 `compile.bat` 确保最新代码已编译。

### 3. 日志查看

打开 Output Log 窗口，过滤 `LogStage` 和 `LogTriggerZone` 查看状态变化。

---

## 测试案例 1：预设行为 - LoadZone

**目标**：验证 `OnEnterAction = Load Stage` 自动触发 Stage 加载。

### 步骤

1. **放置 Stage**
   - Place Actors → 搜索 `Stage`
   - 放置一个 `AStage` Actor
   - 设置 `StageDataLayerAsset`（可选，如果有的话）
   - 记录 Stage 名称（如 `Stage_01`）

2. **放置 TriggerZoneActor**
   - Place Actors → 搜索 `TriggerZone`
   - 放置一个 `ATriggerZoneActor`
   - 位置：放在 Stage 外围，玩家会先经过的地方

3. **配置 TriggerZoneActor**
   - 在 Details 面板中设置：
     ```
     TriggerZone|Binding
     └─ Owner Stage: [选择 Stage_01]

     TriggerZone|Action
     └─ On Enter Action: Load Stage    ← 关键设置
     └─ On Exit Action: Custom (默认)
     ```

4. **运行测试 (PIE)**
   - 点击 Play
   - 控制玩家走进 TriggerZone
   - 观察 Output Log

### 预期结果

```
LogTriggerZone: TriggerZone [TriggerZoneActor_0]: Actor 'BP_PlayerCharacter_C_0' entered
LogTriggerZone: TriggerZone [TriggerZoneActor_0]: Executing preset action -> LoadStage()
LogStage: Stage [Stage_01]: GotoState(Loaded) requested
LogStage: Stage [Stage_01]: State transition 0 -> 1
```

### 验证点

- [ ] 进入 Zone 时自动触发 LoadStage
- [ ] Stage 状态从 Unloaded → Preloading → Loaded
- [ ] 不需要蓝图代码

---

## 测试案例 2：预设行为 - ActivateZone

**目标**：验证 `OnEnterAction = Activate Stage` 自动触发 Stage 激活。

### 步骤

1. **使用案例 1 的 Stage**

2. **放置第二个 TriggerZoneActor**
   - 位置：放在 Stage 核心区域（比 LoadZone 更靠内）
   - 配置：
     ```
     Owner Stage: [Stage_01]
     On Enter Action: Activate Stage    ← 关键设置
     ```

3. **运行测试**
   - 先进入 LoadZone（Stage 变为 Loaded）
   - 再进入 ActivateZone
   - 观察 Output Log

### 预期结果

```
LogStage: Stage [Stage_01]: GotoState(Active) requested
LogStage: Stage [Stage_01]: State transition 2 -> 4
```

### 验证点

- [ ] 从 Loaded → Active 正确转换
- [ ] 如果直接进入 ActivateZone（跳过 LoadZone），应自动完成加载再激活

---

## 测试案例 3：多入口场景 (多个 LoadZone)

**目标**：验证多个 TriggerZone 可以绑定同一个 Stage。

### 场景设计

```
        [LoadZone_North]
              │
              ▼
    ┌─────────────────┐
    │                 │
[LoadZone_West] →  Stage_01  ← [LoadZone_East]
    │                 │
    └─────────────────┘
              │
        [ActivateZone]
```

### 步骤

1. **放置 3 个 LoadZone**
   - 分别放在 Stage 的北、西、东三个方向
   - 每个都设置：
     ```
     Owner Stage: Stage_01
     On Enter Action: Load Stage
     ```

2. **放置 1 个 ActivateZone**
   - 放在 Stage 中心
   - 设置：
     ```
     Owner Stage: Stage_01
     On Enter Action: Activate Stage
     ```

3. **测试各入口**
   - 从北边进入 → 验证 Stage 加载
   - 重启 PIE，从西边进入 → 验证同样效果
   - 重启 PIE，从东边进入 → 验证同样效果

### 验证点

- [ ] 任意入口进入都能触发 Stage 加载
- [ ] 多个 Zone 绑定同一 Stage 无冲突
- [ ] Output Log 显示正确的 Zone 名称

---

## 测试案例 4：Zone 禁用/启用 (PropState 控制)

**目标**：验证 ATriggerZoneActor 的 PropState 控制 Zone 启用状态。

### 步骤

1. **放置 TriggerZoneActor**
   - 配置 Owner Stage 和 On Enter Action

2. **在 Details 面板中找到 Prop 设置**
   ```
   Prop
   └─ Prop State: 0    ← 设置为 0 表示禁用
   ```

3. **运行测试**
   - 进入 Zone，观察是否触发

4. **修改 PropState**
   - 设置 `Prop State: 1`
   - 重新测试

### 预期结果

- PropState = 0：Zone 禁用，进入不触发任何事件
- PropState ≠ 0：Zone 启用，正常触发

### 验证点

- [ ] PropState = 0 时 Zone 不响应
- [ ] PropState 改变后立即生效

---

## 测试案例 5：自定义蓝图逻辑

**目标**：验证 `OnEnterAction = Custom` 时可以使用蓝图事件。

### 步骤

1. **创建蓝图子类**
   - Content Browser → 右键 → Blueprint Class
   - 父类搜索 `TriggerZoneActor`
   - 命名为 `BP_CustomTriggerZone`

2. **编辑蓝图**
   - 打开 BP_CustomTriggerZone
   - 在 Event Graph 中：
     ```
     Event OnActorEnter (Zone, Actor)
         │
         ├─► Print String: "Custom logic triggered!"
         │
         └─► Get Owner Stage → GotoState (Active)
     ```

3. **放置并配置**
   - 放置 BP_CustomTriggerZone
   - 设置：
     ```
     Owner Stage: Stage_01
     On Enter Action: Custom (Blueprint)    ← 使用蓝图逻辑
     ```

4. **运行测试**

### 预期结果

- 进入 Zone 时打印 "Custom logic triggered!"
- Stage 被激活

### 验证点

- [ ] OnActorEnter 事件被调用
- [ ] 可以在蓝图中访问 Stage 并调用 API
- [ ] Custom 模式不会自动执行预设行为

---

## 测试案例 6：Exit 行为 (UnloadZone)

**目标**：验证 `OnExitAction` 在离开 Zone 时触发。

### 步骤

1. **配置 TriggerZoneActor**
   ```
   Owner Stage: Stage_01
   On Enter Action: Activate Stage
   On Exit Action: Unload Stage    ← 离开时卸载
   ```

2. **运行测试**
   - 进入 Zone → Stage 激活
   - 离开 Zone → Stage 卸载

### 预期结果

```
[进入]
LogStage: Stage [Stage_01]: GotoState(Active) requested

[离开]
LogTriggerZone: TriggerZone [...]: Executing preset action -> UnloadStage()
LogStage: Stage [Stage_01]: GotoState(Unloaded) requested
```

### 验证点

- [ ] Exit 事件正确触发
- [ ] Stage 从 Active → Unloading → Unloaded

---

## 测试案例 7：Zone 注册验证

**目标**：验证 TriggerZone 自动注册到 Stage。

### 步骤

1. **放置并配置 Stage 和多个 TriggerZone**

2. **在 Stage 的 Details 面板中查看**
   - 搜索 `Trigger` 相关属性
   - 或者在蓝图中调用 `Stage->GetAllTriggerZones()`

3. **运行时验证**
   - 在 Output Log 中查看注册日志：
     ```
     LogStage: Stage [Stage_01]: Registered TriggerZone [TriggerZoneActor_0] (Total: 1)
     LogStage: Stage [Stage_01]: Registered TriggerZone [TriggerZoneActor_1] (Total: 2)
     ```

### 验证点

- [ ] BeginPlay 时 Zone 自动注册
- [ ] Stage 可以查询所有注册的 Zone
- [ ] EndPlay 时 Zone 自动注销

---

## 测试案例 8：过滤器测试 (TriggerActorTags)

**目标**：验证只有特定 Tag 的 Actor 能触发 Zone。

### 步骤

1. **配置 TriggerZoneActor**
   ```
   TriggerZone|Filtering
   └─ Trigger Actor Tags: ["Player"]    ← 只允许 Player tag
   └─ Require Pawn: false
   ```

2. **给玩家角色添加 Tag**
   - 打开玩家蓝图
   - 在 Class Defaults → Actor → Tags 中添加 "Player"

3. **运行测试**
   - 玩家进入 Zone → 应该触发
   - 其他 Actor（如 NPC）进入 → 不应触发

### 验证点

- [ ] 有 Tag 的 Actor 触发 Zone
- [ ] 无 Tag 的 Actor 不触发
- [ ] 空 Tag 数组时默认只允许 Pawn

---

## 快速检查清单

### 基础功能
- [ ] TriggerZoneActor 可以放置
- [ ] Owner Stage 可以正确设置
- [ ] On Enter/Exit Action 下拉可选

### 预设行为
- [ ] Load Stage 预设工作
- [ ] Activate Stage 预设工作
- [ ] Unload Stage 预设工作
- [ ] Custom 模式不自动执行

### 状态管理
- [ ] GotoState(Loaded) 正确转换
- [ ] GotoState(Active) 正确转换
- [ ] GotoState(Unloaded) 正确转换

### 日志输出
- [ ] LogTriggerZone 显示进入/离开事件
- [ ] LogStage 显示状态转换
- [ ] 预设行为执行有明确日志

---

## 常见问题

### Q: Zone 不触发？

1. 检查 `bZoneEnabled` 是否为 true
2. 检查 PropState 是否 ≠ 0
3. 检查 Owner Stage 是否设置
4. 检查 Collision Profile 是否为 Trigger
5. 检查 Output Log 中的 Warning

### Q: Stage 状态不变？

1. 检查 DataLayer 是否配置（可选）
2. 检查是否有状态锁（bIsStageStateLocked）
3. 查看完整的 LogStage 日志

### Q: 蓝图事件不触发？

1. 确认 On Enter Action = Custom
2. 确认蓝图中绑定了 OnActorEnter 事件
3. 确认事件签名正确 (Zone, Actor)
