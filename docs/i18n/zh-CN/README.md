# README / 项目说明

## 摘要
qt-llmlite 是一个面向 Qt/C++ 的轻量级 LLM 接入库，既可以作为 Qt 包复用，也可以作为源码模块集成。

## 项目介绍
当前版本已从基础聊天客户端扩展为面向产品化的会话与工具编排基础设施，重点能力包括：

- 多客户端工厂与按 `clientId` 持久化恢复
- 单 client 多历史会话管理
- profile 驱动上下文注入（system prompt、capability、preference、style）
- tools 抽象运行时（registry/selection/adapter/execution/policy/catalog）
- `QtLLMClient` 内建 tool loop 状态机与失败保护
- 面向 OpenAI `Responses API` 的独立 `OpenAIProvider`
- 面向 Ollama 等 `/chat/completions` 风格服务的 `OpenAICompatibleProvider`
- MCP Server 注册、持久化、能力查看、工具同步与执行路由
- 统一日志系统：按 `clientId` 轮换 JSONL 文件并推送到 Qt 界面
- 示例程序：`simple_chat`、`multi_client_chat`、`mcp_server_manager_demo`

## 集成方式
- 作为独立 Qt 包引入
- 作为源码模块直接集成

## 主要文档
- `PROJECT_SPEC.md`
- `ARCHITECTURE.md`
- `RELEASE_READINESS_CHECKLIST.md`
- `AI_RULES.md`
- `CODING_GUIDELINES.md`
