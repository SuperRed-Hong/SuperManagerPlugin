# Obsolete 文档说明

此目录存放已废弃的设计方案和开发日志。

## 废弃原因

### Phase1_Metadata_V1.md & Phase2_StatusDetection_V1.md

**废弃日期**: 2025-11-29

**原方案**: 创建独立的 `UStageDataLayerSyncSubsystem` + `FDataLayerImportMetadata` 存储 DataLayer 导入元数据。

**问题**:
1. `UDataLayerAsset` 不支持 `UAssetUserData`，导致需要创建独立 Subsystem 作为 workaround
2. 架构越来越复杂，增加了冗余的存储层
3. 与现有 `UStageEditorSubsystem` 的 Stage 注册表功能重复

**新方案**: 使用反向查找（Reverse Lookup）机制：
- 利用现有的 `AStage::StageDataLayerAsset` 和 `FAct::AssociatedDataLayer` 关系
- 在 `UStageEditorSubsystem` 中添加 `FindStageByDataLayer()` 方法
- 实时对比检测同步状态，无需持久化元数据

详见新文档: `../Architecture_ReverseLookup.md`
