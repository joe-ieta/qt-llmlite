# qt-llm

`qt-llm` is a Qt/C++ LLM integration project centered on the `qtllm` static library. The repository is no longer only a basic chat demo; it now includes the core library, tool runtime, MCP integration, observability, and several desktop applications built around those subsystems.

To reduce ambiguity from older documents, this repository now uses one naming convention:
- repository: `qt-llm`
- library target: `qtllm`
- `qt-llmlite`: historical alias, no longer the primary name

## Current Capabilities

- Base request/response orchestration through `QtLLMClient`
- Multi-client and multi-session conversation management through `ConversationClient` and `ConversationClientFactory`
- Provider abstraction with dedicated OpenAI and OpenAI-compatible paths
- Tool selection, protocol adaptation, execution, failure guard, and follow-up loops
- MCP server registration, persistence, tool sync, and execution routing
- `toolsinside` trace and artifact analysis subsystem
- `toolstudio` tool catalog and workspace management subsystem
- Five application entry points under `src/apps/`

## Recommended Reading Order

1. [README.md](./README.md)
2. [PROJECT_SPEC.md](./PROJECT_SPEC.md)
3. [ARCHITECTURE.md](./ARCHITECTURE.md)
4. [docs/REPOSITORY_STRUCTURE.md](./docs/REPOSITORY_STRUCTURE.md)
5. [docs/DELIVERY_INTEGRATION.md](./docs/DELIVERY_INTEGRATION.md)
6. [docs/TESTING_BASELINE.md](./docs/TESTING_BASELINE.md)
7. [docs/developer-guide/README.md](./docs/developer-guide/README.md)

## Architecture Snapshot

```text
Apps / UI
  -> orchestration entry
     -> ToolEnabledChatEntry          (optional)
     -> ConversationClient
     -> QtLLMClient
  -> Provider abstraction
     -> ILLMProvider
     -> HttpExecutor / StreamChunkParser
  -> runtime subsystems
     -> tools runtime / MCP
     -> logging
     -> toolsinside
     -> storage
```

Main runtime paths:
- base chat: `ConversationClient -> QtLLMClient -> ILLMProvider -> HttpExecutor`
- tool flow: `ToolEnabledChatEntry -> ToolSelectionLayer -> QtLLMClient -> ToolCallOrchestrator -> ToolExecutionLayer`
- MCP flow: `McpServerManager -> McpToolSyncService -> LlmToolRegistry -> ToolExecutionLayer`
- observability flow: `ConversationClient / QtLLMClient / ToolExecutionLayer -> toolsinside`

## Code Entry Points

- core library: `src/qtllm/`
- applications: `src/apps/`
- tests: `tests/qtllm_tests/`
- docs: `docs/`

## Runtime Workspace Data

The project writes runtime state under `.qtllm/`, including:
- `.qtllm/clients/`: conversation snapshots and client-level persistence
- `.qtllm/tools/`: tool catalog, policy, and Tool Studio workspaces
- `.qtllm/mcp/`: MCP server definitions
- `.qtllm/logs/`: per-client JSONL runtime logs
- `.qtllm/tools_inside/`: trace indexes, artifacts, and archive data

## Documentation Governance

The root documents and the main files under `docs/` are now treated as the primary documentation entry points. The translation mirror under `docs/i18n/` remains available, but it is no longer the first place to look for current architecture and scope.
