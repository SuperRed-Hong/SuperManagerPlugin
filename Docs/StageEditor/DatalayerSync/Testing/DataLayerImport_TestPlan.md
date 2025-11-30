# DataLayer 导入功能测试流程文档

## 概述

本文档提供 DataLayer 导入功能的完整测试流程指南，包含测试环境准备、功能测试步骤、预期结果验证及 Bug 报告模板。

---

## 1. 测试环境准备

### 1.1 前置条件

| 条件 | 要求 |
|------|------|
| 引擎版本 | UE 5.6 |
| 插件 | StageEditor 插件已启用 |
| 关卡 | 已打开包含 DataLayer 的关卡 |
| DataLayer | 已创建符合命名规范的 DataLayer |

### 1.2 命名规范

DataLayer 导入功能依赖特定的命名规范：

| 类型 | 命名格式 | 示例 |
|------|----------|------|
| Stage DataLayer | `DL_Stage_<StageName>` | `DL_Stage_Forest` |
| Act DataLayer | `DL_Act_<StageName>_<ActName>` | `DL_Act_Forest_Day` |

**规则**：
- Act DataLayer 必须是对应 Stage DataLayer 的直接子层
- StageName 和 ActName 不能包含下划线

### 1.3 测试数据准备

创建以下测试用 DataLayer 结构：

```
DL_Stage_TestStage           (Stage DataLayer)
├── DL_Act_TestStage_Act1    (Act DataLayer - 包含3个Actor)
├── DL_Act_TestStage_Act2    (Act DataLayer - 包含2个Actor)
└── DL_Act_TestStage_Act3    (Act DataLayer - 空)
```

**创建步骤**：
1. 打开 World Partition 编辑器
2. 创建新的 DataLayerAsset：`DL_Stage_TestStage`
3. 创建子 DataLayer：`DL_Act_TestStage_Act1`、`DL_Act_TestStage_Act2`、`DL_Act_TestStage_Act3`
4. 在场景中放置 Actor 并分配到对应的 Act DataLayer

---

## 2. 功能测试流程

### 2.1 打开 DataLayer 浏览器

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 在编辑器菜单栏选择 **Stage Editor > DataLayer Browser** | 打开 Stage DataLayer Browser 窗口 |
| 2 | 检查窗口标题和布局 | 显示 DataLayer 树形列表，底部有操作按钮 |

### 2.2 命名规范验证测试

#### Test-NR-01：Stage DataLayer 识别

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 在浏览器中查看 `DL_Stage_TestStage` | 显示为可导入状态（NotImported/蓝色图标） |
| 2 | 选中该 DataLayer | "Import Selected" 按钮启用 |

#### Test-NR-02：Act DataLayer 识别

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 在浏览器中查看 `DL_Act_TestStage_Act1` | 显示为 Act 级别，不可单独导入 |
| 2 | 选中该 DataLayer | "Import Selected" 按钮禁用 |

#### Test-NR-03：无效命名拒绝

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 创建名为 `DL_Invalid_Test` 的 DataLayer | 创建成功 |
| 2 | 在浏览器中查看该 DataLayer | 不显示可导入图标 |
| 3 | 选中该 DataLayer 点击 Import | 按钮禁用或显示错误提示 |

---

### 2.3 导入预览测试

#### Test-IP-01：预览对话框显示

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 选中 `DL_Stage_TestStage` | 选中高亮 |
| 2 | 点击 "Import Selected" 按钮 | 打开导入预览对话框 |
| 3 | 检查对话框内容 | 显示树形结构预览 |

#### Test-IP-02：预览层级结构

预览对话框应显示以下结构：

```
▼ Stage: TestStage [SUID: S:?]
  ├── Act: Act1 [SUID: A:?.1] - 3 Props
  ├── Act: Act2 [SUID: A:?.2] - 2 Props
  └── Act: Act3 [SUID: A:?.3] - 0 Props
```

| 验证点 | 预期结果 |
|--------|----------|
| Stage 名称 | 从 `DL_Stage_TestStage` 提取 "TestStage" |
| Act 名称 | 从 `DL_Act_TestStage_Act1` 提取 "Act1" |
| SUID 格式 | Stage 显示 `S:?`，Act 显示 `A:?.N` |
| Props 计数 | 正确显示每个 Act 中的 Actor 数量 |

#### Test-IP-03：预览统计信息

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 查看预览对话框底部 | 显示统计信息 |
| 2 | 验证数字 | Stage: 1, Acts: 3, Props: 5 |

---

### 2.4 导入执行测试

#### Test-IE-01：确认导入

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 在预览对话框点击 "Import" | 对话框关闭，执行导入 |
| 2 | 检查 Outliner | 新建 AStage Actor 名为 "TestStage" |
| 3 | 选中 Stage Actor 检查 Details | 显示 3 个 Acts |

#### Test-IE-02：取消导入

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 在预览对话框点击 "Cancel" | 对话框关闭 |
| 2 | 检查 Outliner | 无新建 Actor |

#### Test-IE-03：SUID 分配验证

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 导入成功后选中 Stage Actor | 检查 StageID 属性 |
| 2 | 验证 StageID | 获得有效 SUID（非0） |
| 3 | 检查每个 Act 的 ActID | 按顺序分配：1, 2, 3 |

#### Test-IE-04：Prop 注册验证

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 在 Stage Editor Panel 中展开 TestStage | 显示 3 个 Acts |
| 2 | 展开 Act1 | 显示 3 个 Props |
| 3 | 点击任意 Prop | 选中场景中对应的 Actor |

#### Test-IE-05：Undo/Redo 支持

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 执行导入操作 | 导入成功 |
| 2 | 按 Ctrl+Z 执行 Undo | 撤销导入，Stage Actor 被删除 |
| 3 | 按 Ctrl+Y 执行 Redo | 重做导入，Stage Actor 恢复 |

---

### 2.5 同步状态检测测试

#### Test-SS-01：NotImported 状态

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 创建新的 `DL_Stage_NewStage` | 创建成功 |
| 2 | 在浏览器中查看 | 显示蓝色图标（NotImported） |
| 3 | 鼠标悬停查看 Tooltip | 显示 "未导入" 信息 |

#### Test-SS-02：Synced 状态

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 导入 `DL_Stage_TestStage` | 导入成功 |
| 2 | 在浏览器中查看 | 显示绿色图标（Synced） |
| 3 | 鼠标悬停查看 Tooltip | 显示 "已同步" 信息 |

#### Test-SS-03：OutOfSync 检测 - 新增 Actor

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 导入 `DL_Stage_TestStage` | 导入成功，显示 Synced |
| 2 | 在场景中创建新 Actor | Actor 创建成功 |
| 3 | 将新 Actor 分配到 `DL_Act_TestStage_Act1` | 分配成功 |
| 4 | 刷新浏览器 | 对应 DataLayer 显示橙色（OutOfSync） |

#### Test-SS-04：OutOfSync 检测 - 删除 Actor

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 从 `DL_Act_TestStage_Act1` 中删除一个 Actor | 删除成功 |
| 2 | 刷新浏览器 | DataLayer 显示 OutOfSync |

#### Test-SS-05：OutOfSync 检测 - 新增子 DataLayer

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 创建 `DL_Act_TestStage_Act4` 作为子层 | 创建成功 |
| 2 | 刷新浏览器 | `DL_Stage_TestStage` 显示 OutOfSync |

---

### 2.6 同步执行测试

#### Test-SE-01：同步单个 DataLayer

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 触发 OutOfSync 状态 | DataLayer 显示橙色 |
| 2 | 选中该 DataLayer | 选中高亮 |
| 3 | 点击 "Sync" 按钮 | 执行同步 |
| 4 | 检查状态 | 变为 Synced（绿色） |

#### Test-SE-02：同步新增 Act

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 创建子 DataLayer `DL_Act_TestStage_NewAct` | 创建成功 |
| 2 | 执行同步 | 同步成功 |
| 3 | 检查 Stage Actor | 新增名为 "NewAct" 的 Act |

#### Test-SE-03：同步新增 Props

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 向 Act DataLayer 添加新 Actor | 添加成功 |
| 2 | 执行同步 | 同步成功 |
| 3 | 检查对应 Act | 新 Actor 被注册为 Prop |

#### Test-SE-04：批量同步

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 使多个 DataLayer 处于 OutOfSync | 多个显示橙色 |
| 2 | 点击 "Sync All" 按钮 | 执行批量同步 |
| 3 | 检查所有状态 | 全部变为 Synced |

---

### 2.7 边界情况测试

#### Test-EC-01：空 Stage 导入

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 创建无子 DataLayer 的 `DL_Stage_Empty` | 创建成功 |
| 2 | 执行导入 | 导入成功，创建无 Act 的 Stage |

#### Test-EC-02：空 Act 导入

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 创建无 Actor 的 Act DataLayer | 创建成功 |
| 2 | 执行导入 | 导入成功，Act 的 Props 为空 |

#### Test-EC-03：重复导入拒绝

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 导入 `DL_Stage_TestStage` | 导入成功 |
| 2 | 再次选中并点击 Import | 按钮禁用或显示错误提示 |
| 3 | 验证错误信息 | "该 DataLayer 已被导入" |

---

### 2.8 错误处理测试

#### Test-EH-01：无 World 错误

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 关闭所有关卡 | 无活动关卡 |
| 2 | 尝试打开 DataLayer 浏览器 | 显示错误或浏览器为空 |

#### Test-EH-02：无效 DataLayer 引用

| 步骤 | 操作 | 预期结果 |
|------|------|----------|
| 1 | 删除已导入 Stage 关联的 DataLayerAsset | 删除成功 |
| 2 | 检查 Stage Editor Panel | 显示警告或提示 |

---

## 3. 测试检查清单

### 3.1 基础功能

- [ ] DataLayer 浏览器正常打开
- [ ] Stage DataLayer 命名识别正确
- [ ] Act DataLayer 命名识别正确
- [ ] 预览对话框正确显示层级结构
- [ ] 导入执行创建正确的 Stage Actor
- [ ] SUID 正确分配
- [ ] Props 正确注册
- [ ] Undo/Redo 支持正常

### 3.2 同步功能

- [ ] NotImported 状态显示正确
- [ ] Synced 状态显示正确
- [ ] OutOfSync 状态检测正确
- [ ] 同步执行正常
- [ ] 批量同步正常

### 3.3 边界情况

- [ ] 空 Stage/Act 处理正确
- [ ] 重复导入被拒绝
- [ ] 错误信息显示清晰

### 3.4 UI 交互

- [ ] 按钮启用/禁用状态正确
- [ ] 状态图标颜色正确
- [ ] Tooltip 信息准确

---

## 4. Bug 报告模板

当发现 Bug 时，请按以下模板填写报告：

### Bug 报告模板

```markdown
# Bug Report: [简短描述]

## 基本信息

| 字段 | 值 |
|------|-----|
| 报告日期 | YYYY-MM-DD |
| 报告人 | [姓名] |
| 引擎版本 | UE 5.6.x |
| 插件版本 | StageEditor v1.x |
| 优先级 | Critical / High / Medium / Low |
| 严重程度 | Blocker / Major / Minor / Trivial |

## Bug 描述

### 简述
[用一句话描述问题]

### 复现步骤
1. [第一步操作]
2. [第二步操作]
3. [第三步操作]
4. ...

### 预期结果
[描述正常情况下应该发生什么]

### 实际结果
[描述实际发生了什么]

## 环境信息

### 测试环境
- 操作系统: Windows 11
- 编辑器模式: Editor / PIE / Standalone
- 关卡名称: [关卡名]
- DataLayer 结构: [描述测试用的 DataLayer 结构]

### 相关文件
- DataLayerAsset: [路径]
- 测试关卡: [路径]

## 附加信息

### 截图/录屏
[附上截图或录屏链接]

### 日志信息
```
[粘贴相关的 Output Log 内容]
```

### 调用栈（如有崩溃）
```
[粘贴崩溃调用栈]
```

## 临时解决方案
[如果有临时绕过方法，请描述]

## 关联测试用例
- Test ID: [关联的测试用例编号，如 Test-IE-01]
```

---

### Bug 优先级定义

| 优先级 | 描述 |
|--------|------|
| **Critical** | 导致崩溃、数据丢失或完全无法使用的问题 |
| **High** | 主要功能无法正常工作，无临时解决方案 |
| **Medium** | 功能受影响但有临时解决方案 |
| **Low** | 轻微问题，不影响主要工作流程 |

### Bug 严重程度定义

| 严重程度 | 描述 |
|----------|------|
| **Blocker** | 阻止继续测试，必须立即修复 |
| **Major** | 严重影响用户体验或数据完整性 |
| **Minor** | 功能可用但表现不理想 |
| **Trivial** | 界面或文字问题，不影响功能 |

---

## 5. Bug 报告示例

### 示例 1：导入后 SUID 未分配

```markdown
# Bug Report: 导入 Stage 后 SUID 始终为 0

## 基本信息

| 字段 | 值 |
|------|-----|
| 报告日期 | 2025-11-29 |
| 报告人 | 张三 |
| 引擎版本 | UE 5.6.0 |
| 插件版本 | StageEditor v1.0 |
| 优先级 | High |
| 严重程度 | Major |

## Bug 描述

### 简述
导入 DataLayer 创建的 Stage Actor 的 StageID 始终为 0，未能正确分配 SUID。

### 复现步骤
1. 创建 DataLayer `DL_Stage_TestBug`
2. 打开 Stage DataLayer Browser
3. 选中 `DL_Stage_TestBug` 并点击 Import
4. 在预览对话框点击确认导入
5. 选中新创建的 Stage Actor 检查 Details Panel

### 预期结果
StageID 应分配一个有效的非零值（如 1、2、3...）

### 实际结果
StageID 显示为 0

## 环境信息

### 测试环境
- 操作系统: Windows 11
- 编辑器模式: Editor
- 关卡名称: TestLevel
- DataLayer 结构: 单个 Stage DataLayer，无子层

## 附加信息

### 截图
[Stage Actor Details Panel 截图显示 StageID = 0]

### 日志信息
```
LogStageEditor: Warning: AssignSUID called but StageRegistry is nullptr
```

## 临时解决方案
手动在 Details Panel 中设置 StageID 值

## 关联测试用例
- Test ID: Test-IE-03
```

---

### 示例 2：预览对话框显示乱码

```markdown
# Bug Report: 中文环境下预览对话框 Act 名称显示乱码

## 基本信息

| 字段 | 值 |
|------|-----|
| 报告日期 | 2025-11-29 |
| 报告人 | 李四 |
| 引擎版本 | UE 5.6.0 |
| 插件版本 | StageEditor v1.0 |
| 优先级 | Medium |
| 严重程度 | Minor |

## Bug 描述

### 简述
当 Act DataLayer 名称包含中文字符时，导入预览对话框中显示乱码。

### 复现步骤
1. 创建 DataLayer `DL_Stage_测试`
2. 创建子 DataLayer `DL_Act_测试_场景1`
3. 打开预览对话框
4. 查看 Act 名称显示

### 预期结果
正确显示 "Act: 场景1"

### 实际结果
显示 "Act: ?????" 或其他乱码字符

## 附加信息

### 截图
[预览对话框截图]

## 临时解决方案
使用英文命名 DataLayer

## 关联测试用例
- Test ID: Test-IP-02
```

---

## 6. 测试报告模板

完成测试后，请填写以下测试报告：

```markdown
# DataLayer 导入功能测试报告

## 测试概要

| 字段 | 值 |
|------|-----|
| 测试日期 | YYYY-MM-DD |
| 测试人员 | [姓名] |
| 测试版本 | StageEditor v1.x |
| 测试环境 | UE 5.6.x / Windows 11 |

## 测试结果汇总

| 类别 | 通过 | 失败 | 跳过 | 总计 |
|------|------|------|------|------|
| 命名规范测试 | X | X | X | X |
| 导入预览测试 | X | X | X | X |
| 导入执行测试 | X | X | X | X |
| 同步状态测试 | X | X | X | X |
| 同步执行测试 | X | X | X | X |
| 边界情况测试 | X | X | X | X |
| 错误处理测试 | X | X | X | X |
| **总计** | **X** | **X** | **X** | **X** |

## 详细测试结果

### 通过的测试用例
- Test-NR-01: Stage DataLayer 识别 ✅
- Test-NR-02: Act DataLayer 识别 ✅
- ...

### 失败的测试用例
| 测试ID | 描述 | Bug ID |
|--------|------|--------|
| Test-IE-03 | SUID 分配验证 | BUG-001 |
| ... | ... | ... |

### 跳过的测试用例
| 测试ID | 描述 | 原因 |
|--------|------|------|
| ... | ... | ... |

## 发现的 Bug 列表

| Bug ID | 描述 | 优先级 | 状态 |
|--------|------|--------|------|
| BUG-001 | SUID 未分配 | High | 待修复 |
| ... | ... | ... | ... |

## 测试结论

[测试总体结论和建议]

## 附录

### 测试日志文件
[附上测试过程中的日志文件]

### 测试环境截图
[测试环境配置截图]
```

---

## 附录 A：测试用例 ID 索引

| ID | 名称 | 分类 |
|----|------|------|
| Test-NR-01 | Stage DataLayer 识别 | 命名规范 |
| Test-NR-02 | Act DataLayer 识别 | 命名规范 |
| Test-NR-03 | 无效命名拒绝 | 命名规范 |
| Test-IP-01 | 预览对话框显示 | 导入预览 |
| Test-IP-02 | 预览层级结构 | 导入预览 |
| Test-IP-03 | 预览统计信息 | 导入预览 |
| Test-IE-01 | 确认导入 | 导入执行 |
| Test-IE-02 | 取消导入 | 导入执行 |
| Test-IE-03 | SUID 分配验证 | 导入执行 |
| Test-IE-04 | Prop 注册验证 | 导入执行 |
| Test-IE-05 | Undo/Redo 支持 | 导入执行 |
| Test-SS-01 | NotImported 状态 | 同步状态 |
| Test-SS-02 | Synced 状态 | 同步状态 |
| Test-SS-03 | OutOfSync - 新增 Actor | 同步状态 |
| Test-SS-04 | OutOfSync - 删除 Actor | 同步状态 |
| Test-SS-05 | OutOfSync - 新增子 DataLayer | 同步状态 |
| Test-SE-01 | 同步单个 DataLayer | 同步执行 |
| Test-SE-02 | 同步新增 Act | 同步执行 |
| Test-SE-03 | 同步新增 Props | 同步执行 |
| Test-SE-04 | 批量同步 | 同步执行 |
| Test-EC-01 | 空 Stage 导入 | 边界情况 |
| Test-EC-02 | 空 Act 导入 | 边界情况 |
| Test-EC-03 | 重复导入拒绝 | 边界情况 |
| Test-EH-01 | 无 World 错误 | 错误处理 |
| Test-EH-02 | 无效 DataLayer 引用 | 错误处理 |
