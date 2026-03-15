# ARCHITECTURE.md

## Overview

qt-llm is organized as a focused Qt library plus reference applications.

The dependency direction remains:

```text
Application/UI
  -> orchestration entry (base or tool-enabled)
  -> QtLLMClient
  -> ILLMProvider
  -> HttpExecutor / Qt network layer
  -> remote or local LLM service
```

`QtLLMClient` is the base client API and can run an internal tool-call loop when tool runtime context is configured.

## Architectural principles

### 1. Provider abstraction first
Provider implementations own vendor request and response mapping. Application code should not embed protocol JSON details.

### 2. Async-first request pipeline
Network execution remains signal-driven and non-blocking.

### 3. Streaming-ready design
Provider interfaces expose both token-level and richer stream delta parsing.

### 4. Additive evolution
Conversation persistence, tool runtime, MCP integration, and logging are additive layers on top of the base chat client.

### 5. Small modules with strict boundaries
Each module keeps narrow responsibility and avoids circular coupling.

## Source layout (current)

```text
src/
  qtllm/
    core/
      qtllmclient.h/.cpp
      llmtypes.h
      llmconfig.h
    chat/
      conversationclient.h/.cpp
      conversationclientfactory.h/.cpp
      conversationsnapshot.h
    profile/
      clientprofile.h
      memorypolicy.h
    storage/
      conversationrepository.h/.cpp
    providers/
      illmprovider.h
      openaiprovider.h/.cpp
      openaicompatibleprovider.h/.cpp
      ollamaprovider.h/.cpp
      vllmprovider.h/.cpp
      providerfactory.h/.cpp
    logging/
      logtypes.h/.cpp
      ilogsink.h
      filelogsink.h/.cpp
      signallogsink.h/.cpp
      qtllmlogger.h/.cpp
    network/
      httpexecutor.h/.cpp
    streaming/
      streamchunkparser.h/.cpp
    tools/
      llmtooldefinition.h
      llmtoolregistry.h/.cpp
      builtintools.h/.cpp
      llmtooladapter.h/.cpp
      toolselectionlayer.h/.cpp
      toolenabledchatentry.h/.cpp
      mcp/
        mcp_types.h
        imcpclient.h
        defaultmcpclient.h/.cpp
        mcpserverregistry.h/.cpp
        mcpserverrepository.h/.cpp
        mcpservermanager.h/.cpp
        mcptoolsyncservice.h/.cpp
      protocol/
        itoolcallprotocoladapter.h
        providerprotocoladapters.h/.cpp
        toolcallprotocolrouter.h/.cpp
      runtime/
        toolruntime_types.h
        toolexecutionlayer.h/.cpp
        toolexecutorregistry.h/.cpp
        itoolexecutor.h
        builtinexecutors.h/.cpp
        toolcallorchestrator.h/.cpp
        toolcatalogrepository.h/.cpp
        clienttoolpolicyrepository.h/.cpp
  examples/
    simple_chat/
      main.cpp
      chatwindow.h/.cpp
    multi_client_chat/
      main.cpp
      multiclientwindow.h/.cpp
    mcp_server_manager_demo/
      main.cpp
      mcpservermanagerwindow.h/.cpp
      mcpchatwindow.h/.cpp
```

## Runtime flows

### A. Base chat flow
1. UI calls `QtLLMClient::sendPrompt(...)` or `sendRequest(...)`
2. Client resolves provider and dispatches through `HttpExecutor`
3. Provider parses full response or stream deltas
4. Client emits `tokenReceived`, `responseReceived`, `completed`, or `errorOccurred`

### B. Conversation flow
1. UI acquires `ConversationClient` by `clientId`
2. Factory loads or saves snapshot data through `ConversationRepository`
3. `ConversationClient` manages one active session and multiple history sessions
4. Before send, `ConversationClient` binds `clientId` and `sessionId` into inner runtime context

### C. Tool-enabled flow
1. App optionally uses `ToolEnabledChatEntry`
2. Entry reads profile, history, tool registry, and client policy
3. `ToolSelectionLayer` picks candidate tools for the current turn
4. `LlmToolAdapter` maps canonical tools into provider-facing tool schema
5. Entry sends the request through `ConversationClient`
6. `QtLLMClient` runs the internal tool loop through `ToolCallOrchestrator`

### D. MCP-backed external tool flow
1. Host app registers MCP servers through `McpServerManager`
2. Server definitions persist under workspace `.qtllm/mcp/servers.json`
3. `McpToolSyncService` lists remote tools and registers them into `LlmToolRegistry`
4. Registered MCP tools participate in normal selection and provider tool-schema generation
5. `ToolExecutionLayer` resolves tool source and routes MCP tools through `IMcpClient`

### E. Logging and observability flow
1. Runtime code writes structured `LogEvent` records through `QtLlmLogger`
2. `FileLogSink` stores per-client rotated JSONL logs under `.qtllm/logs/<clientId>/`
3. `SignalLogSink` pushes the same events into Qt widgets for live inspection

## Design boundaries

### `QtLLMClient`
Should manage:
- active provider and HTTP executor
- request lifecycle and streaming output
- provider payload diagnostics
- optional internal tool-loop state

Should not manage:
- UI rendering
- tool catalog persistence policy
- MCP server registry CRUD

### `ConversationClient` / `ConversationClientFactory`
Should manage:
- client identity lifecycle
- active session and history sessions
- snapshot persistence triggers
- provider/config binding for request construction

Should not manage:
- vendor protocol mapping
- low-level HTTP mechanics

### `ILLMProvider`
Should manage:
- request URLs, headers, and payload construction
- response parsing and stream parsing
- vendor-specific structured tool-call semantics

Should not manage:
- UI behavior
- business policies

### Tools runtime
Should manage:
- canonical tool definitions and registry
- built-in tool bootstrap
- provider protocol routing
- execution abstraction, loop orchestration, and failure guard
- MCP execution routing

Should not manage:
- widget composition
- provider transport details outside tool execution

### MCP module
Should manage:
- MCP server definitions and validation
- persistent server registry
- server capability listing and tool import
- MCP tool execution bridge

Should not manage:
- generic tool-selection policy
- direct UI rendering

### Logging module
Should manage:
- structured event model
- sink registration and filtering by level
- per-client rotated file logging
- Qt signal delivery for UI inspection

Should not manage:
- business decisions based on log content

## Canonical tool model

`LlmToolDefinition` now separates three identifiers:
- `toolId`: internal unique identifier used by registry, policy, persistence, and execution
- `invocationName`: provider-visible function name sent to the model
- `name`: human-readable display name for UI

Each tool also contains:
- `description`
- `inputSchema`
- `capabilityTags`
- metadata such as `category`, built-in/removable/enabled flags

## Persistence model

### Conversation snapshot
Persisted by `clientId`, includes:
- profile
- provider config (`providerName`, `baseUrl`, `apiKey`, `model`, `modelVendor`, `stream`)
- active session id
- session list and message history

### Tool and MCP data
- tool catalog persisted by `ToolCatalogRepository`
- client tool policy persisted by `ClientToolPolicyRepository`
- MCP servers persisted by `McpServerRepository`
- built-ins are always merged and enforced at startup

### Logs
- workspace-local `.qtllm/logs/<clientId>/YYYYMMDD-XXXX.jsonl`
- UTF-8 encoded JSON Lines
- rotated by size, pruned by per-client file-count limit

## Known hardening areas

- broader provider-native protocol coverage
- richer structured tool trace persistence in session history
- more automated coverage for tool-loop retry and guard branches
- structured log filtering/export in more example apps
