# World Partition 转换功能更新

## 📋 问题

之前的自定义 World Partition 转换功能存在严重问题：
- ❌ 缺少很多关键的 World Settings 配置
- ❌ 没有正确设置 External Actors
- ❌ 没有处理 DataLayer 配置
- ❌ 转换后的关卡配置不完整

## ✅ 解决方案

**已将自定义转换功能替换为 UE 原生的 World Partition 转换系统**

### 修改内容

#### 1. `StageEditorController.cpp`
```cpp
void FStageEditorController::ConvertToWorldPartition()
{
    // 使用 UE 原生的 World Partition 转换功能
    FWorldPartitionEditorModule& WorldPartitionEditorModule = 
        FModuleManager::LoadModuleChecked<FWorldPartitionEditorModule>("WorldPartitionEditor");
    
    FString LongPackageName = World->GetPackage()->GetName();
    
    // 调用原生转换函数
    WorldPartitionEditorModule.ConvertMap(LongPackageName);
}
```

#### 2. `StageEditor.Build.cs`
添加了模块依赖：
```csharp
PrivateDependencyModuleNames.AddRange(
    new string[]
    {
        // ... 其他模块 ...
        "WorldPartitionEditor",  // 新增
    }
);
```

---

## 🎯 现在的行为

当您点击 "Convert to World Partition" 按钮时：

### 1. **显示转换对话框** ✅
UE 会显示一个专业的转换设置对话框，包含：
- 转换选项（In-Place 或创建副本）
- 是否合并子关卡
- 是否删除源关卡
- 是否生成 INI 配置
- 详细的转换报告选项

### 2. **运行 WorldPartitionConvertCommandlet** ✅
UE 会在后台运行官方的转换命令行工具，这个工具会：
- 正确配置所有 World Partition 设置
- 自动启用 External Actors
- 正确设置 World Data Layers
- 处理所有子关卡和流式加载
- 生成必要的配置文件

### 3. **完整的配置** ✅
转换后的关卡会包含：
- ✅ World Partition 对象
- ✅ External Actors 配置
- ✅ Use Actor Folder Objects
- ✅ World Data Layers
- ✅ 正确的流式加载设置
- ✅ HLOD 配置（如果需要）

### 4. **自动重新加载关卡** ✅
转换完成后，UE 会：
- 重新扫描资产
- 自动加载转换后的关卡
- 刷新编辑器UI

---

## 📝 使用方法

### 方式 1: 使用 Stage Editor 按钮
1. 打开 Stage Editor 面板
2. 点击 **"Convert to World Partition"** 按钮
3. 在弹出的对话框中配置转换选项
4. 点击 **OK** 开始转换
5. 等待转换完成（会显示进度）

### 方式 2: 使用 UE 原生菜单（等效）
1. 菜单栏 → **Window** → **Convert to World Partition**
2. 选择要转换的关卡
3. 配置转换选项
4. 开始转换

**两种方式完全等效！** 我们的按钮现在直接调用 UE 的原生功能。

---

## ⚠️ 重要提示

### 转换前准备
1. **保存所有修改** - 转换前会提示保存
2. **备份关卡** - 转换是不可逆的
3. **关闭其他编辑器** - 确保没有其他资产编辑器打开

### 转换选项说明

#### In-Place 转换
- ✅ 直接修改当前关卡
- ⚠️ 不可撤销
- 推荐用于新项目

#### 创建副本转换
- ✅ 保留原关卡
- ✅ 创建新的 World Partition 关卡
- 推荐用于生产项目

#### 合并子关卡
- 如果关卡有子关卡，可以选择是否合并
- 合并后所有内容在一个 World Partition 中

---

## 🔧 技术细节

### UE 原生转换流程

1. **保存当前状态**
   ```cpp
   AskSaveDirtyPackages(/*bAskSaveContentPackages=*/false)
   ```

2. **卸载当前关卡**
   ```cpp
   UnloadCurrentMap(LongPackageName)
   ```

3. **运行 Commandlet**
   ```cpp
   RunCommandletAsExternalProcess(CommandletArgs, OperationDescription, Result, bCancelled)
   ```

4. **重新扫描资产**
   ```cpp
   RescanAssets(MapToLoad)
   ```

5. **加载转换后的关卡**
   ```cpp
   LoadMap(MapToLoad)
   ```

### 转换 Commandlet 参数
```
-run=WorldPartitionConvertCommandlet <PackageName> -AllowCommandletRendering
[-ConversionSuffix]           // 创建副本
[-SkipStableGUIDValidation]   // 跳过 GUID 验证
[-DeleteSourceLevels]         // 删除源关卡
[-GenerateIni]                // 生成 INI 配置
[-ReportOnly]                 // 仅生成报告
[-Verbose]                    // 详细日志
[-OnlyMergeSubLevels]         // 仅合并子关卡
```

---

## 📚 相关文档

- [UE World Partition 官方文档](https://docs.unrealengine.com/5.0/en-US/world-partition-in-unreal-engine/)
- [World Partition 转换指南](https://docs.unrealengine.com/5.0/en-US/converting-levels-to-world-partition-in-unreal-engine/)
- `WorldPartitionEditorModule.cpp` (第 747 行 - `ConvertMap` 函数)
- `WorldPartitionConvertCommandlet.cpp` - 转换命令行工具实现

---

## 🎉 优势

### 对比自定义实现

| 特性 | 自定义实现 | UE 原生 |
|------|-----------|---------|
| 配置完整性 | ❌ 缺少很多设置 | ✅ 完整配置 |
| External Actors | ❌ 需要手动启用 | ✅ 自动配置 |
| Data Layers | ❌ 不处理 | ✅ 自动处理 |
| 子关卡合并 | ❌ 不支持 | ✅ 支持 |
| 错误处理 | ❌ 基础 | ✅ 完善 |
| 进度显示 | ❌ 无 | ✅ 详细进度 |
| 日志记录 | ❌ 简单 | ✅ 完整日志 |
| 可撤销 | ❌ 否 | ✅ 可选副本模式 |

---

**更新日期**: 2025-11-22  
**状态**: ✅ 已完成  
**影响文件**:
- `StageEditorController.cpp`
- `StageEditor.Build.cs`
