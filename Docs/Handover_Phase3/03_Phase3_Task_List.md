# 03 - Phase 3 任务清单

## 🎯 Phase 3 目标

创建 `UStageEditorSubsystem` 并实现基于 ID 的 DataLayer 唯一性命名系统。

---

## ✅ 任务清单

### 任务 1: 创建 StageEditorSubsystem

#### 1.1 创建头文件
- [ ] 创建 `Plugins/StageEditor/Source/StageEditor/Public/Subsystems/StageEditorSubsystem.h`
- [ ] 定义类继承自 `UEditorSubsystem`
- [ ] 声明 Stage 注册表 (`TMap<int32, TWeakObjectPtr<AStage>>`)
- [ ] 声明 ID 分配器 (`int32 NextStageID`)
- [ ] 声明公共 API:
  - `RegisterStage(AStage*) → int32`
  - `UnregisterStage(int32)`
  - `GetStage(int32) → AStage*`
  - `GetAllStages() → TArray<AStage*>`
  - `IsStageIDRegistered(int32) → bool`
- [ ] 声明私有方法:
  - `AllocateStageID() → int32`
  - `CleanupRegistry()`

#### 1.2 创建实现文件
- [ ] 创建 `Plugins/StageEditor/Source/StageEditor/Private/Subsystems/StageEditorSubsystem.cpp`
- [ ] 实现 `Initialize()` - 初始化 NextStageID = 1
- [ ] 实现 `Deinitialize()` - 清空注册表
- [ ] 实现 `RegisterStage()` - 分配 ID 并注册
- [ ] 实现 `UnregisterStage()` - 从注册表移除
- [ ] 实现 `GetStage()` - 通过 ID 查找
- [ ] 实现 `GetAllStages()` - 返回所有 Stage
- [ ] 实现 `IsStageIDRegistered()` - 检查 ID 是否存在
- [ ] 实现 `AllocateStageID()` - 递增分配
- [ ] 实现 `CleanupRegistry()` - 清理无效引用

**验收标准**:
- ✅ 代码编译无错误
- ✅ Subsystem 在编辑器启动时初始化
- ✅ 日志显示 "StageEditorSubsystem initialized"

---

### 任务 2: 修改 Stage Actor

#### 2.1 修改 Stage.h
- [ ] 在 `#if WITH_EDITOR` 块中添加:
  - `PostActorCreated()` 声明
  - `PostEditChangeProperty()` 声明（已有，需要更新）
  - `RegisterWithSubsystem()` 私有方法声明
  - `bIsRegistered` 私有成员变量

#### 2.2 修改 Stage.cpp
- [ ] 添加 `#include "Subsystems/StageEditorSubsystem.h"`
- [ ] 实现 `PostActorCreated()`:
  - 调用 `RegisterWithSubsystem()`
- [ ] 更新 `PostEditChangeProperty()`:
  - 添加注册检查
  - 保留现有的 DataLayer 同步逻辑
- [ ] 实现 `RegisterWithSubsystem()`:
  - 获取 Subsystem
  - 如果 StageID <= 0，请求分配
  - 调用 `Subsystem->RegisterStage(this)`
  - 设置 `bIsRegistered = true`
  - 调用 `Modify()` 标记为脏
- [ ] 更新 `BeginDestroy()`:
  - 调用 `Subsystem->UnregisterStage(StageID)`

**验收标准**:
- ✅ 创建 Stage 时自动分配 StageID
- ✅ StageID 从 1 开始递增
- ✅ 日志显示 "Registered Stage with ID: X"
- ✅ 删除 Stage 时正确取消注册

---

### 任务 3: 更新 StageEditorController

#### 3.1 修改 StageEditorController.h
- [ ] 添加私有方法声明:
  - `GetSubsystem() const → UStageEditorSubsystem*`

#### 3.2 修改 StageEditorController.cpp
- [ ] 添加 `#include "Subsystems/StageEditorSubsystem.h"`
- [ ] 实现 `GetSubsystem()`:
  - 返回 `GEditor->GetEditorSubsystem<UStageEditorSubsystem>()`
- [ ] 更新 `CreateDataLayerForStage()`:
  - 检查 `Stage->StageID > 0`
  - 使用 `FString::Printf(TEXT("DL_Stage_%d"), Stage->StageID)`
  - 移除旧的命名逻辑
- [ ] 更新 `CreateDataLayerForAct()`:
  - 检查 `Stage->StageID > 0`
  - 使用 `FString::Printf(TEXT("DL_Act_%d_%d"), Stage->StageID, ActID)`
  - 移除旧的命名逻辑
- [ ] 更新 `CreateNewStage()`:
  - 验证 `NewStage->StageID > 0`
  - 添加错误处理

**验收标准**:
- ✅ DataLayer 名称格式为 `DL_Stage_{StageID}`
- ✅ Act DataLayer 名称格式为 `DL_Act_{StageID}_{ActID}`
- ✅ 创建失败时显示友好错误信息

---

### 任务 4: 测试验证

#### 4.1 单元测试
- [ ] **测试用例 1**: 创建单个 Stage
  - 验证 StageID = 1
  - 验证 Subsystem 中已注册
  - 验证 DataLayer 名称为 `DL_Stage_1`

- [ ] **测试用例 2**: 创建多个 Stage
  - 创建 Stage A (ID 应为 1)
  - 创建 Stage B (ID 应为 2)
  - 创建 Stage C (ID 应为 3)
  - 验证所有 ID 唯一
  - 验证所有 DataLayer 名称唯一

- [ ] **测试用例 3**: 创建 Act
  - 在 Stage (ID=1) 中创建 Act
  - 验证 Act DataLayer 名称为 `DL_Act_1_{ActID}`
  - 验证父子关系正确

- [ ] **测试用例 4**: 删除 Stage
  - 创建 Stage (ID=1)
  - 删除 Stage
  - 验证 Subsystem 中已取消注册
  - 创建新 Stage
  - 验证新 Stage 得到 ID 2

#### 4.2 集成测试
- [ ] 完整工作流测试:
  1. 创建 3 个 Stage
  2. 每个 Stage 创建 2 个 Act
  3. 验证所有 DataLayer 名称唯一
  4. 删除中间的 Stage
  5. 创建新 Stage
  6. 验证 ID 继续递增

**验收标准**:
- ✅ 所有测试用例通过
- ✅ 无编译错误或警告
- ✅ 无运行时错误
- ✅ DataLayer 名称全部基于 ID

---

## 📊 进度跟踪

### 总体进度
- [ ] 任务 1: 创建 StageEditorSubsystem (0/10)
- [ ] 任务 2: 修改 Stage Actor (0/8)
- [ ] 任务 3: 更新 StageEditorController (0/6)
- [ ] 任务 4: 测试验证 (0/6)

**总计**: 0/30 任务完成

### 预计时间
- 任务 1: 30 分钟
- 任务 2: 30 分钟
- 任务 3: 30 分钟
- 任务 4: 30 分钟
- **总计**: 约 2 小时

---

## ✅ 最终验收标准

### 功能验收
- [ ] Subsystem 正确初始化和清理
- [ ] Stage 创建时自动注册
- [ ] StageID 从 1 开始递增
- [ ] DataLayer 名称基于 StageID
- [ ] 多个 Stage 的 ID 和 DataLayer 名称都唯一
- [ ] Stage 删除时正确取消注册
- [ ] 所有测试用例通过

### 代码质量
- [ ] 编译无错误无警告
- [ ] 代码符合项目风格
- [ ] 添加了完整的注释
- [ ] 日志信息清晰有用
- [ ] 错误处理完善

### 文档
- [ ] 更新了任务进度文档
- [ ] 记录了遇到的问题和解决方案
- [ ] 创建了测试报告

---

## 🚨 常见问题和注意事项

### 问题 1: Subsystem 未初始化
**症状**: 获取 Subsystem 返回 nullptr
**解决**: 确保在 `#if WITH_EDITOR` 块中，且编辑器已启动

### 问题 2: StageID 为 0
**症状**: Stage 创建后 StageID 仍为 0
**解决**: 检查 `PostActorCreated` 是否被调用，检查 `RegisterWithSubsystem` 逻辑

### 问题 3: DataLayer 名称未更新
**症状**: DataLayer 仍使用旧的命名方式
**解决**: 确保 `CreateDataLayerForStage` 和 `CreateDataLayerForAct` 已更新

### 问题 4: 编译错误
**症状**: 找不到 `UStageEditorSubsystem`
**解决**: 检查头文件路径，确保 `#include` 正确

---

## 📝 提交检查清单

在提交代码前，请确认：

- [ ] 所有任务已完成
- [ ] 代码已编译通过
- [ ] 所有测试用例已通过
- [ ] 代码已格式化
- [ ] 注释已添加
- [ ] 日志信息已优化
- [ ] 文档已更新
- [ ] 已清理调试代码
- [ ] 已测试 Undo/Redo
- [ ] 已测试保存/加载

---

**按照这个清单逐项完成，Phase 3 就能顺利交付！** ✅
