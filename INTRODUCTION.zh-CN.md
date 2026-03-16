# qt-llmlite 项目介绍（中文）

## 项目定位

`qt-llmlite` 是一个面向 Qt/C++ 应用的轻量级 LLM 接入底层库。

它不是单一聊天 Demo，而是可扩展的基础设施层：

- 多客户端会话管理与持久化（`clientId`）
- 单客户端多历史会话（`active session + historical sessions`）
- profile 驱动上下文（system prompt / persona / 偏好 / 思维风格）
- tools 抽象运行时（registry / selection / adapter / execution / policy / catalog）
- `QtLLMClient` 内建 tool loop 状态机与失败保护
- 面向 OpenAI `Responses API` 的独立 `OpenAIProvider`
- 面向 Ollama 等 `/chat/completions` 风格服务的 `OpenAICompatibleProvider`
- MCP Server 注册、持久化、工具同步、能力查看与执行路由
- 按 `clientId` 轮换落盘并可推送到界面的统一日志系统
- 示例程序：`simple_chat`、`multi_client_chat`、`mcp_server_manager_demo`

## 核心价值

- 低侵入接入：对现有 Qt 工程改造成本可控
- 演进清晰：可从基础聊天平滑升级到工具编排能力
- 抽象稳定：保持 Provider 边界，便于后续扩展模型厂商
- 可观测性增强：便于排查 Provider、工具调用和 MCP 链路问题

## 入口文档

- [首页 README](./README.md)
- [项目规格](./PROJECT_SPEC.md)
- [架构说明](./ARCHITECTURE.md)
- [发布前检查清单](./RELEASE_READINESS_CHECKLIST.md)
