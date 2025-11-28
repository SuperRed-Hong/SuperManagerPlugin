# DataLayer Asset Path Configuration - Walkthrough

## 概述 (Overview)
本次更新为 DataLayer 资产创建添加了路径配置功能，使其与 Stage 和 Prop 蓝图的路径配置保持一致。用户现在可以选择使用默认路径或自定义路径来保存 DataLayer 资产。

## 更改内容 (Changes Made)

### 1. FAssetCreationSettings (StageEditorPanel.h)
- **添加了 `bIsCustomDataLayerAssetPath`**: 布尔标志，控制是否使用自定义路径
- **添加了 `DataLayerAssetFolderPath`**: 自定义文件夹路径配置
- **默认路径**: `/StageEditor/DataLayers`

```cpp
/** If true, use custom path for DataLayer Assets. Otherwise use default plugin path. */
UPROPERTY(EditAnywhere, Category = "Asset Creation")
bool bIsCustomDataLayerAssetPath = false;

/** Custom folder path for DataLayer Asset creation */
UPROPERTY(EditAnywhere, Category = "Asset Creation", meta = (EditCondition = "bIsCustomDataLayerAssetPath", ContentDir, RelativeToGameContentDir))
FDirectoryPath DataLayerAssetFolderPath;
```

### 2. StageEditorController (StageEditorController.h / .cpp)
- **添加了 `SetDataLayerAssetFolderPath`**: 设置 DataLayer 资产文件夹路径
- **添加了 `GetDataLayerAssetFolderPath`**: 获取当前配置的路径
- **添加了私有成员变量 `DataLayerAssetFolderPath`**: 存储配置的路径，默认值为 `/StageEditor/DataLayers`
- **更新了 `CreateDataLayerForStage`**: 使用配置的路径而不是默认参数
- **更新了 `CreateDataLayerForAct`**: 使用配置的路径而不是默认参数

### 3. StageEditorPanel (StageEditorPanel.cpp)
- **初始化时设置路径**: 在构造函数中根据用户配置初始化 Controller 的 DataLayer 路径
- **实现了 `OnAssetCreationSettingsChanged`**: 当用户在 UI 中更改设置时，自动更新 Controller 的路径配置
- **路径转换逻辑**: 支持物理路径到虚拟路径的转换

## 工作原理 (How It Works)

### 默认模式
当 `bIsCustomDataLayerAssetPath = false` 时：
- 所有 DataLayer 资产保存到 `/StageEditor/DataLayers/`
- 这是推荐的默认配置，便于统一管理

### 自定义模式
当 `bIsCustomDataLayerAssetPath = true` 时：
- 用户可以在 UI 中选择自定义文件夹
- 路径会自动转换为虚拟路径格式（如 `/Game/MyFolder/`）
- 所有新创建的 DataLayer 资产将保存到指定位置

### 动态更新
- 用户在 Stage Editor Panel 的 "Asset Creation Settings" 区域更改设置
- `OnAssetCreationSettingsChanged` 回调被触发
- Controller 的路径配置立即更新
- 后续创建的 DataLayer 资产使用新路径

## 验证 (Verification)
- ✅ 编译成功
- ✅ 与 Stage/Prop 蓝图路径配置保持一致的 UI 模式
- ✅ 支持默认路径和自定义路径两种模式
- ✅ 实时响应用户配置更改

## 使用示例 (Usage Example)

### 使用默认路径
1. 打开 Stage Editor 面板
2. "Asset Creation Settings" 中保持 "Is Custom Data Layer Asset Path" 为 false
3. 创建 Stage 或 Act 时，DataLayer 资产自动保存到 `/StageEditor/DataLayers/`

### 使用自定义路径
1. 打开 Stage Editor 面板
2. 勾选 "Is Custom Data Layer Asset Path"
3. 选择自定义文件夹（如 `Content/MyProject/DataLayers`）
4. 创建 Stage 或 Act 时，DataLayer 资产保存到指定位置

## 技术细节 (Technical Details)

### 为什么之前不需要传入 FolderPath 参数？
在 `CreateDataLayerAsset` 方法中，`FolderPath` 参数有默认值：
```cpp
UDataLayerAsset* CreateDataLayerAsset(
    const FString& AssetName, 
    const FString& FolderPath = TEXT("/StageEditor/DataLayers")
);
```

这是 C++ 的默认参数特性，调用时可以省略该参数，编译器会自动使用默认值。

### 现在的改进
现在我们显式传入配置的路径，使其可以根据用户设置动态调整：
```cpp
UDataLayerAsset* StageDataLayerAsset = CreateDataLayerAsset(AssetName, DataLayerAssetFolderPath);
```

这样既保持了灵活性，又提供了用户可配置的选项。
