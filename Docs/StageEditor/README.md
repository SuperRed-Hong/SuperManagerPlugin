# StageEditor 文档导航

> **StageEditor** 是一个基于 DataLayer 的动态关卡管理系统，使用戏剧隐喻（Stage → Acts → Props）实现灵活的场景状态控制。

---

## 📖 开发日志

- **[Overview.md](Overview.md)** - 完整开发总览，涵盖所有 Phase 的进度和状态

---

## 🏗️ 核心架构

### Phase 13: StageRegistry 持久化架构重设计
- **[Phase13_StageRegistry_Discussion.md](CoreArchitecture/Phase13_StageRegistry_Discussion.md)** - 架构设计讨论文档
  - StageID 持久化问题与解决方案
  - 双层架构（DataAsset + Subsystem）
  - LevelInstance 支持
  - 多人协作方案

### 其他架构讨论
- **[RecentActivatedActID语义讨论.md](CoreArchitecture/RecentActivatedActID语义讨论.md)**
- **[SUID唯一性设计讨论.md](CoreArchitecture/SUID唯一性设计讨论.md)**

---

## 🔗 DataLayer 集成功能

### 开发文档（按 Phase 编号）

**Phase 1-2: 反向查找与性能优化**
- [Phase1-2_ReverseLookup.md](DataLayerIntegration/Phase1-2_ReverseLookup.md) - 反向查找实现
- [Phase1-2_PerformanceOptimization.md](DataLayerIntegration/Phase1-2_PerformanceOptimization.md) - 缓存层优化

**Phase 3-7: 核心功能**
- [Phase3_Parser.md](DataLayerIntegration/Phase3_Parser.md) - 命名解析模块
- [Phase4_UI.md](DataLayerIntegration/Phase4_UI.md) - DataLayerOutliner UI
- [Phase5_Import.md](DataLayerIntegration/Phase5_Import.md) - 导入逻辑与预览对话框
- [Phase6_Sync.md](DataLayerIntegration/Phase6_Sync.md) - 同步逻辑
- [Phase7_Localization.md](DataLayerIntegration/Phase7_Localization.md) - 本地化（中英文）

**Phase 8: UI 扩展（SceneOutliner 框架）**
- [Phase8_UI_Extension_Research.md](DataLayerIntegration/Phase8_UI_Extension_Research.md) - UI 扩展预研
- [Phase8_1_SceneOutliner_Foundation.md](DataLayerIntegration/Phase8_1_SceneOutliner_Foundation.md) - 基础架构
- [Phase8_2_CustomColumns.md](DataLayerIntegration/Phase8_2_CustomColumns.md) - 自定义列实现
- [Phase8_3_Integration.md](DataLayerIntegration/Phase8_3_Integration.md) - 集成与测试
- [Phase8_4_NativeFeatures.md](DataLayerIntegration/Phase8_4_NativeFeatures.md) - 原生功能迁移

**Phase 9-12: 优化与增强**
- [Phase10_ImportRedesign.md](DataLayerIntegration/Phase10_ImportRedesign.md) - Import 功能重设计
- [Phase11_CacheEventDriven.md](DataLayerIntegration/Phase11_CacheEventDriven.md) - 缓存事件驱动优化

### 架构设计文档
- [Architecture_ReverseLookup.md](DataLayerIntegration/Architecture_ReverseLookup.md) - 反向查找架构方案
- [Architecture_Integration_Analysis.md](DataLayerIntegration/Architecture_Integration_Analysis.md) - 架构整合分析

### 其他
- [UI_Development.md](DataLayerIntegration/UI_Development.md) - UI 开发总览
- [README.md](DataLayerIntegration/README.md) - DataLayer 集成功能说明
- [Testing/](DataLayerIntegration/Testing/) - 测试计划

---

## 📋 规格文档

- **[TechSpec.md](Specs/TechSpec.md)** - 技术规格文档（v4.0）
- **[PRD.md](Specs/PRD.md)** - 产品需求文档
- **[FeatureSpecs/](Specs/FeatureSpecs/)** - 功能规格文档
  - [DataLayerOutlinerExtension_Spec.md](Specs/FeatureSpecs/DataLayerOutlinerExtension_Spec.md)

---

## 🎮 蓝图 API

- **[BlueprintAPI/StageEditorBlueprintAPI.md](BlueprintAPI/StageEditorBlueprintAPI.md)** - 蓝图 API 完整文档

---

## 📅 实施计划

- **[ImplementationPlan/](ImplementationPlan/)** - 实施计划与 Todo List

---

## 🗄️ 废弃文档

- **[Obsolete/](Obsolete/)** - 历史废弃文档存档
  - [DataLayerIntegration/](Obsolete/DataLayerIntegration/) - DataLayer 集成相关废弃文档

---

## 🚀 快速开始

1. **了解项目概况** → 阅读 [Overview.md](Overview.md)
2. **查看当前开发** → Phase 13: [StageRegistry 持久化](CoreArchitecture/Phase13_StageRegistry_Discussion.md)
3. **理解核心功能** → [DataLayer 集成文档](DataLayerIntegration/)
4. **查阅技术规格** → [TechSpec.md](Specs/TechSpec.md)

---

*文档结构更新: 2025-12-04*
*StageEditor 版本: Phase 13 开发中*
