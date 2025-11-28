# Claude 生成的指南文档

这个文件夹包含 Claude Code 在开发过程中自动生成的技术文档和指南。

## 📂 文件夹说明

- **目的**: 存放 Claude 辅助生成的文档，与项目原有设计文档分开管理
- **内容**: 技术分析、使用指南、问题排查、最佳实践等
- **更新**: 随开发过程动态更新

---

## 📄 当前文档列表

### DataLayer 相关
- `DataLayer_WorkingPrinciple.md` - DataLayer 在 World Partition vs 传统 Level 中的工作原理
- `TraditionalLevel_Enhancement.md` - 传统 Level 下的增强方案（如需支持）

### World Partition 相关
- `WorldPartition_Enforcement_Guide.md` - World Partition 强制要求功能使用指南
- `WorldPartition_MapCheck_Errors.md` - MapCheck 错误原因分析和修复方案

### 编辑器功能
- `SelectionSync_Analysis.md` - StageEditor Panel 与 SceneOutliner 选择同步机制分析

---

## 🔄 与原有文档的区别

| 文件夹 | 内容 | 作者 | 用途 |
|--------|------|------|------|
| `Docs/StageEditor/` | 设计文档、PRD、架构设计 | 项目团队 | 开发设计参考 |
| `Docs/ClaudeGuides/` | 技术指南、使用说明、问题排查 | Claude 生成 | 开发辅助、问题解决 |

---

## 📝 文档维护

**添加新文档:**
- Claude 生成的所有指南类文档都应放在此文件夹
- 命名规范: `主题_说明.md` (使用英文下划线分隔)

**文档分类建议:**
```
Docs/ClaudeGuides/
├── DataLayer_*.md          # DataLayer 相关
├── WorldPartition_*.md     # World Partition 相关
├── Selection_*.md          # 选择同步相关
├── Troubleshooting_*.md    # 问题排查
└── BestPractices_*.md      # 最佳实践
```

---

## 🎯 使用建议

**何时查看这些文档:**
1. 遇到 DataLayer 相关问题
2. World Partition 转换后的错误
3. 需要了解选择同步机制
4. 寻找最佳实践和设计决策说明

**何时查看原有文档 (`Docs/StageEditor/`):**
1. 了解系统整体架构
2. 查看需求和设计规格
3. 理解 MVC 模式实现
4. 查看中文设计文档

---

## ⚡ 快速导航

**常见问题:**
- MapCheck 错误 → `WorldPartition_MapCheck_Errors.md`
- 选择同步问题 → `SelectionSync_Analysis.md`
- DataLayer 不工作 → `DataLayer_WorkingPrinciple.md`

**功能说明:**
- World Partition 要求 → `WorldPartition_Enforcement_Guide.md`
- 传统 Level 支持 → `TraditionalLevel_Enhancement.md`
