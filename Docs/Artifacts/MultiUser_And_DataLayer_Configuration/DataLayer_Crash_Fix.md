# DataLayer 崩溃问题修复指南

## 🔴 问题描述

执行 "Add Act" 操作时 UE 崩溃，错误信息：
```
Assertion failed: GetLevel()->IsUsingExternalObjects() 
[File: WorldDataLayers.cpp] [Line: 714]
```

## 🔍 根本原因

**关卡没有启用 External Objects（外部对象）功能**，这是 World Partition DataLayer 系统的必需配置。

### 为什么会崩溃？

1. DataLayer Asset 创建成功 ✅
2. 尝试创建 DataLayerInstance 时检查关卡配置
3. 发现关卡不支持外部对象
4. 断言失败 → 崩溃 ❌

## ✅ 解决方案

### 方案 1: 启用 External Actors（推荐）

#### 步骤 1: 打开 World Settings
1. 在编辑器中，点击菜单 **Window → World Settings**
2. 或者在 World Outliner 中点击 **World Settings** 按钮

#### 步骤 2: 启用 External Actors
在 World Settings 面板中：
1. 找到 **World Partition** 部分
2. 勾选 **"Use External Actors"** 或 **"Enable Streaming"**
3. 如果看到 "Convert to World Partition" 按钮，点击它

#### 步骤 3: 保存关卡
- **File → Save Current Level** 或 Ctrl+S

#### 步骤 4: 重新测试
- 关闭并重新打开编辑器
- 尝试添加 Act
- 应该能正常创建 DataLayer 了

---

### 方案 2: 使用代码安全检查（已实现）

我已经在代码中添加了安全检查，现在如果关卡不支持外部对象，会：

1. **不会崩溃** ✅
2. **显示友好的错误提示** ✅
3. **在日志中记录详细信息** ✅

#### 修改内容

在 `GetOrCreateInstanceForAsset` 方法中添加了检查：

```cpp
// Check if level supports external objects (required for World Partition DataLayers)
ULevel* Level = World->PersistentLevel;
if (!Level || !Level->IsUsingExternalObjects())
{
    UE_LOG(LogTemp, Error, TEXT("❌ Cannot create DataLayerInstance: Level does not support External Objects."));
    UE_LOG(LogTemp, Error, TEXT("   Please enable 'Use External Actors' in World Settings → World Partition."));
    DebugHeader::ShowNotifyInfo(TEXT("Error: Level must have 'Use External Actors' enabled for DataLayer creation."));
    return nullptr;
}
```

#### 现在的行为

当您点击 "Add Act" 时：
- ✅ Act 会被创建
- ✅ DataLayer Asset 会被创建
- ❌ DataLayer Instance 创建失败（但不会崩溃）
- 📢 显示通知："Error: Level must have 'Use External Actors' enabled for DataLayer creation."

---

## 📋 完整操作流程

### 第一次设置（必须）

1. **关闭 Unreal Editor**
2. **重新编译项目**
   ```bash
   d:\UEProject\ReserchPJ\ExtendEditor\ExtendEditor\compile.bat
   ```
3. **打开编辑器**
4. **启用 External Actors**（见方案 1）
5. **保存关卡**

### 日常使用

启用 External Actors 后，DataLayer 功能将正常工作：

1. **创建 Stage**
   - 自动创建 `DL_Stage_{StageName}` 资产
   - 自动创建对应的 DataLayerInstance

2. **添加 Act**
   - 自动创建 `DL_Act_{StageName}_{ActID}_{ActName}` 资产
   - 自动创建对应的 DataLayerInstance
   - 自动设置为 Stage DataLayer 的子层

3. **注册 Props**
   - 自动分配到 Stage 的 DataLayer

---

## 🔧 验证配置

### 检查关卡是否支持 External Objects

在编辑器中打开 Output Log，查找：
```
LogTemp: ❌ Cannot create DataLayerInstance: Level does not support External Objects.
```

如果看到这条消息，说明需要启用 External Actors。

### 检查 DataLayer 是否创建成功

在 Content Browser 中查看：
- 默认路径：`/Game/StageEditor/DataLayers/`
- 或您配置的自定义路径

应该能看到：
- `DL_Stage_XXX.uasset` - Stage 的 DataLayer
- `DL_Act_XXX.uasset` - Act 的 DataLayer

---

## 📚 相关文档

- [Unreal Engine World Partition 文档](https://docs.unrealengine.com/5.0/en-US/world-partition-in-unreal-engine/)
- [DataLayer 系统文档](https://docs.unrealengine.com/5.0/en-US/world-partition-data-layers-in-unreal-engine/)

---

## ❓ 常见问题

### Q: 为什么之前这个功能被注释掉了？
A: 就是因为这个 External Objects 的问题。之前遇到崩溃后临时禁用了自动创建功能。

### Q: 不启用 External Actors 可以吗？
A: 不可以。World Partition 的 DataLayer 系统**必须**在支持外部对象的关卡中使用。

### Q: 如何判断关卡是否是 World Partition 类型？
A: 在 World Settings 中查看是否有 "World Partition" 部分。如果没有，需要先转换为 World Partition 关卡。

### Q: 转换为 World Partition 会影响现有内容吗？
A: 可能会。建议在转换前备份关卡。转换是**不可逆**的操作。

---

**更新日期**: 2025-11-22  
**状态**: ✅ 已修复（添加了安全检查）  
**下一步**: 启用 External Actors 配置
