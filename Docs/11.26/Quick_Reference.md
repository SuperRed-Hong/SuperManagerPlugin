# 快速参考 - DataLayer 和 World Partition 配置

## 🚀 快速开始

### 第一次使用 Stage Editor 的 DataLayer 功能

#### 步骤 1: 转换关卡为 World Partition
1. 打开 Stage Editor 面板
2. 点击 **"Convert to World Partition"** 按钮
3. 在对话框中选择：
   - ✅ 创建副本（推荐，保留原关卡）
   - ✅ 生成 INI
4. 点击 **OK** 开始转换
5. 等待转换完成

#### 步骤 2: 启用 External Objects
1. 打开 **Window → World Settings**
2. 找到 **World Partition** 部分
3. 勾选 **"Use Actor Folder Objects"**
4. 勾选 **"Use External Actors"**
5. **保存关卡** (Ctrl+S)
6. **重启编辑器**

#### 步骤 3: 测试 DataLayer 创建
1. 打开 Stage Editor 面板
2. 创建一个 Stage（如果还没有）
3. 点击 **"Add Act"** 按钮
4. 检查 Content Browser 中的 `/Game/StageEditor/DataLayers/` 文件夹
5. 应该能看到新创建的 DataLayer 资产

---

## ⚠️ 常见问题快速解决

### 问题 1: 点击 "Add Act" 没有创建 DataLayer

**可能原因**:
- 关卡不是 World Partition 类型
- 没有启用 External Objects

**解决方法**:
1. 检查关卡是否是 World Partition（World Settings 中应该有 "World Partition" 部分）
2. 如果不是，使用 "Convert to World Partition" 按钮
3. 确保启用了 "Use External Actors"

### 问题 2: 看到错误 "Level does not support External Objects"

**解决方法**:
1. World Settings → World Partition
2. 勾选 "Use Actor Folder Objects"
3. 勾选 "Use External Actors"
4. 保存关卡
5. 重启编辑器

### 问题 3: DataLayer 创建了但是是黑色的

**原因**: 使用了旧版本代码

**解决方法**:
1. 更新到最新代码
2. 删除旧的 DataLayer 资产
3. 重新创建

### 问题 4: UE 崩溃

**原因**: 使用了旧版本代码（已修复）

**解决方法**:
1. 更新到最新代码
2. 重新编译
3. 现在会显示友好的错误提示而不是崩溃

---

## 📋 配置检查清单

### World Partition 配置 ✅

- [ ] 关卡已转换为 World Partition
- [ ] World Settings 中有 "World Partition" 部分
- [ ] "Use Actor Folder Objects" 已勾选
- [ ] "Use External Actors" 已勾选
- [ ] 关卡已保存
- [ ] 编辑器已重启

### DataLayer 功能验证 ✅

- [ ] 创建 Stage 成功
- [ ] 创建 Act 成功
- [ ] DataLayer 资产自动创建
- [ ] DataLayer 有颜色（不是黑色）
- [ ] DataLayer 类型为 Runtime
- [ ] Props 可以注册到 Stage

---

## 🔧 关键设置位置

### World Settings
- **位置**: Window → World Settings
- **关键选项**:
  - World Partition
    - ✅ Use Actor Folder Objects
    - ✅ Use External Actors
    - ✅ Enable World Bounds Checks（可选）

### Stage Editor Panel
- **位置**: Window → Stage Editor
- **关键按钮**:
  - Convert to World Partition
  - Create Stage
  - Add Act
  - Register Props

### Asset Creation Settings
- **位置**: Stage Editor Panel → Asset Creation Settings
- **关键选项**:
  - Stage Blueprint Folder Path
  - Prop Blueprint Folder Path
  - DataLayer Asset Folder Path
    - ✅ Use Custom Path（可选）
    - 默认: `/Game/StageEditor/DataLayers/`

---

## 📂 文件位置

### DataLayer 资产
- **默认路径**: `/Game/StageEditor/DataLayers/`
- **命名规则**:
  - Stage: `DL_Stage_{StageName}`
  - Act: `DL_Act_{StageName}_{ActID}_{ActName}`

### External Actors
- **路径**: `Content/__ExternalActors__/{LevelName}/`
- **说明**: 启用 External Actors 后，所有 Actor 会被移动到这里

### World Data Layers
- **路径**: `Content/__ExternalObjects__/{LevelName}/`
- **说明**: World Partition 的 DataLayer 配置文件

---

## 🎯 下一步工作

### 立即任务
1. [ ] 编译项目
2. [ ] 测试 World Partition 转换
3. [ ] 配置 External Objects
4. [ ] 测试 DataLayer 创建

### 后续任务
1. [ ] Phase 3: DataLayer Creation Uniqueness
2. [ ] Phase 4: UI Display Name Handling
3. [ ] Phase 5: Backward Compatibility
4. [ ] Phase 6: Testing

---

## 📚 详细文档

### 本次会话文档
- `Session_Summary.md` - 完整的会话总结
- `Quick_Reference.md` - 本文件

### 之前的文档
- `Docs\Artifacts\MultiUser_And_DataLayer_Configuration\README.md`
- `Docs\Artifacts\MultiUser_And_DataLayer_Configuration\DataLayer_Crash_Fix.md`
- `Docs\Artifacts\MultiUser_And_DataLayer_Configuration\WorldPartition_Conversion_Update.md`
- `Docs\Artifacts\MultiUser_And_DataLayer_Configuration\Task_Progress.md`

---

## 💡 提示

### 最佳实践
1. ✅ 总是先转换为 World Partition
2. ✅ 总是启用 External Actors
3. ✅ 定期保存关卡
4. ✅ 使用版本控制
5. ✅ 备份重要数据

### 避免的操作
1. ❌ 不要在非 World Partition 关卡中使用 DataLayer 功能
2. ❌ 不要忘记启用 External Actors
3. ❌ 不要在转换过程中关闭编辑器
4. ❌ 不要手动修改 External Actors 文件夹

---

**最后更新**: 2025-11-26  
**版本**: 1.0
