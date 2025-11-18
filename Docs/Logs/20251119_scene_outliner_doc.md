# scene outliner doc

## 问题描述
- 具体问题：需要一份 Scene Outliner 插件开发速查文档
- 影响范围：希望扩展 Outliner 的所有开发者

## 解决方案
- 技术方案：在 Docs/Guides 添加 SceneOutlinerDevGuide.md，梳理接口上下文、关键 API、开发步骤
- 修改的文件：Docs/Guides/SceneOutlinerDevGuide.md

## 实施步骤
1. 整理 ISceneOutliner、Selection、Column 等接口并写入文档

## 测试验证
- （文档任务）不适用
- 测试结果：文档已生成

## 注意事项
- 潜在风险：SceneOutliner API 可能随版本调整，需定期校对 Docs/UESource
- 后续优化：可后续补更多示例或 UML

## 问题提醒
- 首次开发 Outliner 插件容易忽略 Mode/Selection 限制，需对照文档与源码

## 下一步建议
- 结合文档绘制思维导图，熟悉各接口位置
