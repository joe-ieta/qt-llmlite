# ARCHITECTURE / 架构说明

## 摘要
qt-llm 由聚焦的 Qt 核心库与示例应用组成，围绕 Provider 抽象、会话持久化、工具运行时、MCP 集成和统一日志展开。

## 依赖流向
`Application/UI -> 编排入口 -> QtLLMClient -> ILLMProvider -> HttpExecutor -> LLM 服务`

## 关键模块
- `QtLLMClient`：请求生命周期、流式输出、内部 tool loop
- `ConversationClient`：client/session 生命周期与持久化绑定
- `OpenAIProvider`：OpenAI `/responses`
- `OpenAICompatibleProvider`：`/chat/completions` 与映射厂商协议
- tools runtime：registry、selection、protocol adapter、execution、orchestrator
- MCP 模块：server registry、持久化、sync、执行桥接
- logging 模块：file sink、signal sink、统一 logger

## 工具标识拆分
- `toolId`：内部路由与持久化
- `invocationName`：provider 可见函数名
- `name`：界面展示名

## 工作区运行时数据
- `.qtllm/mcp/servers.json`
- `.qtllm/tools/...`
- `.qtllm/logs/<clientId>/...jsonl`
