# ARCHITECTURE / Architecture

## Summary
qt-llm is a focused Qt library plus example apps, built around provider abstraction, conversation persistence, tool runtime, MCP integration, and structured logging.

## Dependency Flow
`Application/UI -> orchestration entry -> QtLLMClient -> ILLMProvider -> HttpExecutor -> LLM service`

## Key Modules
- `QtLLMClient`: request lifecycle, streaming, internal tool loop
- `ConversationClient`: client/session lifecycle and persistence binding
- `OpenAIProvider`: OpenAI `/responses`
- `OpenAICompatibleProvider`: `/chat/completions` and mapped vendor schemas
- tools runtime: registry, selection, protocol adapter, execution, orchestrator
- MCP module: server registry, persistence, sync, execution bridge
- logging module: file sink, signal sink, unified logger

## Tool Identity Split
- `toolId` for internal routing and persistence
- `invocationName` for provider-facing function calls
- `name` for UI display

## Workspace State
- `.qtllm/mcp/servers.json`
- `.qtllm/tools/...`
- `.qtllm/logs/<clientId>/...jsonl`
