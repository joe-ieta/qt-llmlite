# ARCHITECTURE.md

## Overview

qt-llm is structured as a small Qt library plus example applications.

The dependency direction remains:

```text
Application/UI
  -> orchestration entry (base or tool-enabled)
  -> QtLLMClient
  -> ILLMProvider
  -> HttpExecutor / Qt network layer
  -> remote or local LLM service
```

`QtLLMClient` is still the base client API and now includes an internal tool-loop state path when orchestrator/context are configured.

## Architectural principles

### 1. Provider abstraction first

The client should not directly carry vendor JSON protocol logic; provider implementations map protocol details.

### 2. Async-first request pipeline

Network calls remain non-blocking and signal-driven.

### 3. Streaming-ready design

Interfaces anticipate streamed output and fragmented chunks.

### 4. Additive evolution

Factory persistence, tools, policies, and protocol routing are additive layers around base chat capabilities.

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
      openaicompatibleprovider.h/.cpp
      ollamaprovider.h/.cpp
      vllmprovider.h/.cpp
      providerfactory.h/.cpp
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
```

## Runtime flows

### A. Base text/tool-capable flow

1. UI calls `QtLLMClient::sendPrompt(...)` or `sendRequest(...)`
2. Client resolves provider and dispatches request through `HttpExecutor`
3. Provider parses response or stream chunks
4. If tool orchestrator/context are configured:
   - parse tool calls (structured first, text fallback)
   - execute via runtime layer
   - build follow-up prompt/message
   - continue until completion or failure guard
5. Client emits `tokenReceived`, `responseReceived`, `completed`, `errorOccurred`

### B. Multi-client/session flow

1. UI acquires client from `ConversationClientFactory` by `clientId`
2. Factory loads/saves snapshot through `ConversationRepository`
3. `ConversationClient` keeps one active session and multiple history sessions
4. UI can create/switch sessions by `sessionId`
5. Before send, `ConversationClient` binds tool-loop context (`clientId`, `sessionId`) to inner `QtLLMClient`

### C. Tool-enabled entry flow

1. App optionally uses `ToolEnabledChatEntry`
2. Entry reads profile/history/tool registry and policy repository
3. Selection layer picks candidate tools
4. Adapter maps canonical tools to provider/model schema hints
5. Entry injects orchestrator/policy wiring and sends message through `ConversationClient`
6. Internal `QtLLMClient` loop executes tool-call chain

## Design boundaries

### `QtLLMClient`

Should manage:

- active provider and executor
- request lifecycle
- stream token emission
- optional internal tool-loop state flow

Should not manage:

- tool registration/catalog persistence concerns
- UI rendering
- high-level product policies

### `ConversationClient` / `ConversationClientFactory`

Should manage:

- client identity lifecycle
- per-client active session and session history
- snapshot persistence triggers
- profile/config binding for request construction

Should not manage:

- vendor protocol JSON details
- low-level HTTP mechanics

### `ILLMProvider`

Should manage:

- URLs, headers, payload mapping
- response parsing
- stream token parsing
- vendor schema adaptation

Should not manage:

- UI behavior
- top-level scheduling/policy

### `HttpExecutor`

Should manage:

- `QNetworkAccessManager`
- HTTP execution lifecycle
- timeout/retry/cancel

Should not manage:

- provider/tool parsing semantics

### Tools runtime

Should manage:

- canonical tool definitions and registry
- built-in tool bootstrap and immutability
- protocol adapter routing (vendor-first)
- execution layer abstraction and policy checks
- loop orchestration with failure guard
- tool catalog and client policy persistence

Should not manage:

- replacing base chat APIs
- widget-level logic

## Canonical tool model

`LlmToolDefinition` includes:

- `toolId`, `name`, `description`
- `inputSchema`
- `capabilityTags`
- metadata (`category`, built-in/removable/enabled flags)

Provider adapters map canonical definitions to vendor-specific descriptors.

## Persistence model

### Conversation snapshot

Persisted by `clientId`, includes:

- profile
- provider config (`providerName`, `baseUrl`, `apiKey`, `model`, `modelVendor`, `stream`)
- active session id
- session list and message history

### Tools data

- tool catalog persisted by `ToolCatalogRepository`
- client tool policy persisted by `ClientToolPolicyRepository`
- built-ins are always merged/enforced at startup

## Known gaps and next hardening

- strict provider-native protocol coverage for more vendors
- richer structured tool trace persistence in session history
- broader runtime tests for loop guard/retry/error branches
- stronger observability and audit fields for production operations
