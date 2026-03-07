# DECISIONS / 设计决策

## 摘要 / Summary
qt-llmlite 的关键架构决策如下。

## 决策 1：qmake 优先
基线采用 qmake，以匹配 Qt Creator 的主流使用路径。

## 决策 2：OpenAI 兼容基线
Provider 抽象优先对齐 OpenAI 兼容 HTTP 格式，提升通用性。

## 决策 3：本地优先
项目明确支持本地/内网模型服务（如 Ollama、vLLM）。

## 决策 4：轻量架构
初始阶段避免重依赖，优先建立清晰可扩展骨架。

## 决策 5：AI 友好文档
保留结构化上下文文档，降低人类与 AI 协作成本。