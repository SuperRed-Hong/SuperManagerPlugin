# Phase 7: 本地化（中英文）

> 完成日期: 2025-11-29
> 状态: ✅ 完成（框架就位）

---

## 目标

确保所有 UI 文本支持国际化，为后续中文翻译做好准备。

---

## 实现内容

### 1. LOCTEXT 宏使用

所有 UI 文本均使用 `LOCTEXT` 宏包装：

```cpp
#define LOCTEXT_NAMESPACE "StageEditorDataLayerSync"

// 示例用法
LOCTEXT("StatusSynced", "Synced - No changes detected")
LOCTEXT("StatusNotImported", "Not Imported - Click Import to add to StageEditor")
LOCTEXT("ImportButton", "Import")
LOCTEXT("SyncAllButton", "Sync All")

#undef LOCTEXT_NAMESPACE
```

### 2. 命名空间分布

| 模块 | LOCTEXT_NAMESPACE |
|------|-------------------|
| DataLayerImporter | `StageEditorDataLayerImport` |
| DataLayerSynchronizer | `StageEditorDataLayerSync` |
| DataLayerSyncStatus | `StageEditorDataLayerSync` |
| SStageDataLayerBrowser | `StageDataLayerBrowser` |
| SDataLayerImportPreviewDialog | `StageEditorDataLayerImport` |

### 3. 已本地化的文本

**导入相关：**
- `DialogTitle` - "Import DataLayer as Stage"
- `SourceLabel` - "Source: {0}"
- `WillCreateLabel` - "Will Create:"
- `ImportButton` - "Import"
- `CancelButton` - "Cancel"
- `SummaryFormat` - "Total: {0} Stage, {1} Acts, {2} Props"

**同步相关：**
- `StatusSynced` - "Synced - No changes detected"
- `StatusNotImported` - "Not Imported - Click Import to add to StageEditor"
- `NewChildDetected` - "New child detected: {0}"
- `ChildRemoved` - "Child removed: {0}"
- `ActorChanges` - "Actor changes: +{0} / -{1}"
- `ClickToSync` - "Click Sync to update"

**浏览器相关：**
- `SyncAllButton` - "Sync All"
- `SyncAllTooltip` - "Synchronize all out-of-sync DataLayers..."
- `ImportSelectedButton` - "Import Selected"
- `ImportSelectedTooltip` - "Import selected Stage DataLayer..."
- `NamingHint` - "Use DL_Stage_<name> or DL_Act_<stage>_<name> naming"

---

## 后续步骤（中文翻译）

### 使用 UE Localization Dashboard

1. **打开 Localization Dashboard**
   - 窗口 → Localization Dashboard

2. **配置 Localization Target**
   - 添加 `StageEditor` 目标
   - 添加 `zh-Hans`（简体中文）文化

3. **收集文本**
   - 点击 "Gather Text"
   - 选择 C++ 源代码路径

4. **导出翻译**
   - 导出 `.po` 文件
   - 翻译后导入

5. **编译本地化**
   - 点击 "Compile"
   - 生成 `.locres` 文件

### 翻译示例

| 英文 | 中文 |
|------|------|
| Import DataLayer as Stage | 将 DataLayer 导入为 Stage |
| Synced - No changes detected | 已同步 - 未检测到变化 |
| Not Imported | 未导入 |
| Click Sync to update | 点击同步以更新 |
| New child detected: {0} | 检测到新子项: {0} |

---

## 技术要点

### LOCTEXT vs NSLOCTEXT

- `LOCTEXT(Key, Default)` - 使用当前 `LOCTEXT_NAMESPACE`
- `NSLOCTEXT(Namespace, Key, Default)` - 显式指定命名空间

建议在大多数情况下使用 `LOCTEXT`，在模块边界使用 `NSLOCTEXT`。

### 格式化文本

```cpp
FText::Format(LOCTEXT("SummaryFormat", "Total: {0} Stage, {1} Acts, {2} Props"),
    FText::AsNumber(StageCount),
    FText::AsNumber(ActCount),
    FText::AsNumber(PropCount));
```

---

*完成日期: 2025-11-29 17:20*
