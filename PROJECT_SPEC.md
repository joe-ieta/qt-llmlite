# PROJECT_SPEC.md

## 1. Project Overview

**Project Name**

qt-llmlite (qt-llm)

**Purpose**

qt-llmlite is a lightweight Qt/C++ toolkit for integrating local-first and remote Large Language Model services into Qt applications.

Primary support targets:
- Qt desktop applications
- local LLM services
- remote LLM services through dedicated vendor APIs or OpenAI-compatible APIs

---

## 2. Design Goals

1. Qt-native implementation
2. lightweight architecture
3. minimal external dependencies
4. provider-agnostic abstraction
5. local-first capability
6. async network communication
7. streaming token support
8. easy integration into existing Qt apps
9. multi-client conversation orchestration with persistence
10. tool/function-calling capability with production-oriented extension points
11. MCP server registration, external tool sync, and execution routing
12. structured observability with file logging and UI log streaming

---

## 3. Technical Direction

- Language: C++17
- Framework: Qt
- Build system: qmake
- IDE target: Qt Creator
- Main transport: HTTP-based APIs
- OpenAI path: dedicated `Responses API` provider
- OpenAI-compatible path: `/chat/completions`

Constraints:
- keep core library Python-free
- avoid heavy frameworks and runtime dependencies

---

## 4. Provider Types

### Local providers
- Ollama
- vLLM
- llama.cpp-compatible servers
- LM Studio-compatible endpoints

### Remote providers
- OpenAI
- Anthropic
- Google
- DeepSeek / Qwen / GLM-class providers through mapped compatible schemas

Provider logic remains outside application UI and is encapsulated by `ILLMProvider` implementations.

---

## 5. High-Level Architecture

```text
Qt Application/UI
  -> (A) QtLLMClient                        [base client + built-in tool loop]
  -> (B) ToolEnabledChatEntry               [optional orchestration entry]
        -> tool selection + schema adapter + policy
        -> QtLLMClient / Provider abstraction
  -> ILLMProvider
  -> HttpExecutor
  -> Local or remote LLM service
```

Supporting modules:
- configuration model
- request/response types
- async HTTP execution
- streaming chunk handling
- conversation client factory and persistence
- tool registry / catalog / policy / execution / protocol routing
- MCP server registry / persistence / client bridge
- structured logging sinks and logger manager

---

## 6. Core Components

### 6.1 QtLLMClient
- receives prompts or structured requests from app code
- owns active provider and executor
- exposes async API via signals and slots
- emits streaming content and reasoning deltas
- emits structured response events
- includes built-in tool-loop handling with failure guard
- publishes final provider URL and payload for diagnostics

### 6.2 ConversationClient + ConversationClientFactory
- creates and manages multiple client identities (`clientId`)
- persists and restores client conversation state
- tracks one active session and multiple historical sessions per client
- supports session switching and history replay
- binds `clientId` and `sessionId` into request and tool-loop context

### 6.3 ILLMProvider
- provider identity
- request URL, headers, and payload generation
- final response parsing
- stream token and delta parsing
- capability signal for structured tool calls

### 6.4 HttpExecutor
- wraps `QNetworkAccessManager`
- emits `dataReceived`, `requestFinished`, `errorOccurred`
- keeps request flow async and UI-thread friendly

### 6.5 Tool/function-calling abstraction
- `LlmToolRegistry` maintains the total tool set
- `LlmToolDefinition` is the canonical internal tool model
- built-in non-removable tools are auto-registered on startup
- `ToolSelectionLayer` chooses tools per turn
- `ToolCallProtocolRouter` routes parsing/serialization by provider protocol
- `ToolExecutionLayer` dispatches built-in versus MCP-backed execution
- `ToolCallOrchestrator` handles parsing, execution, follow-up, and loop guard
- `ToolEnabledChatEntry` is the optional app-facing wiring point

### 6.6 MCP integration
- `McpServerManager` provides host-app CRUD over registered MCP servers
- `McpServerRepository` persists server definitions under workspace `.qtllm/mcp/servers.json`
- `IMcpClient` and `DefaultMcpClient` support `stdio`, `streamable-http`, and `sse`
- `McpToolSyncService` imports registered MCP tools into `LlmToolRegistry`
- MCP tools execute through the normal tool loop and are routed by tool source

### 6.7 Logging and diagnostics
- `QtLlmLogger` is the unified logging entry point
- `FileLogSink` writes per-client rotated JSONL logs under workspace `.qtllm/logs/`
- `SignalLogSink` pushes live log events to Qt widgets
- log events carry `clientId`, `sessionId`, `requestId`, category, level, and structured fields

### 6.8 Configuration model
- `providerName`, `baseUrl`, `apiKey`, `model`, `modelVendor`, `stream`
- request control fields such as timeout/retry remain in `LlmConfig`

---

## 7. Current Implementation Status (as of 2026-03-15)

### Completed
- qmake multi-project workspace
- `QtLLMClient` async orchestration with internal tool loop
- `ConversationClientFactory` multi-client persistence architecture
- dedicated `OpenAIProvider` for `/responses`
- `OpenAICompatibleProvider` for `/chat/completions` plus mapped Anthropic/Google variants
- provider aliases for OpenAI-compatible vendors
- `HttpExecutor` async POST wrapper
- tool runtime stack:
  - registry, built-ins, schema adapter, selection layer
  - protocol adapters/router
  - execution layer, orchestrator, catalog/policy repositories
- canonical tool identity split: `toolId`, `invocationName`, `name`
- MCP stack:
  - server registry/repository/manager
  - tool sync service
  - MCP execution routing
- example apps:
  - `simple_chat`
  - `multi_client_chat`
  - `mcp_server_manager_demo`
- unified logging module with file rotation and UI signal delivery

### Partial / pending hardening
- broader automated tests for MCP, OpenAI Responses API events, and loop guard edge cases
- richer structured tool trace persistence in session history
- stricter provider-native coverage for more vendors
- production-grade async execution for long-running external tools

---

## 8. Scope Baseline (Current)

Current baseline includes:
- reusable core library (`src/qtllm`)
- multi-client/session persistence foundation
- dedicated OpenAI and OpenAI-compatible provider paths
- canonical tool registry with built-in and MCP-imported tools
- MCP server management and persistence
- structured runtime logging
- demo Qt Widgets applications (`simple_chat`, `multi_client_chat`, `mcp_server_manager_demo`)
- documentation set (English + Chinese)

---

## 9. Roadmap (Next)

### Phase 2: protocol and runtime hardening
- expand provider-native protocol coverage and tests
- improve fragmented stream behavior for more providers
- harden timeout/retry/cancel inside tool-loop chains

### Phase 3: conversation and trace quality
- persist structured tool-call/tool-result traces
- improve session replay and debugging quality
- add structured log filtering/export helpers in shared UI components

### Phase 4: production controls
- permission and safety policies per tool category and server source
- stronger observability fields and audit trails
- model/tool routing optimization based on capability and latency

---

## 10. Constraints

- must work on Windows and Linux
- should stay friendly to Qt Creator users
- should support local and intranet-hosted LLM servers
- should keep compile and dependency overhead low
- should not assume cloud-only usage

---

## 11. Non-goals

The project will not:
- implement model inference itself
- embed heavyweight serving frameworks
- tightly couple UI code with provider internals

---

## 12. Instructions for Coding Agents

1. Read `PROJECT_SPEC.md`, `AI_RULES.md`, `ARCHITECTURE.md`, and `CODING_GUIDELINES.md` first.
2. Preserve provider abstraction boundaries.
3. Keep networking asynchronous.
4. Prefer incremental, compilable changes.
5. Avoid unnecessary dependencies.
6. Keep Qt Creator workflow first-class.
7. Keep tool, MCP, and logging changes additive and backward compatible for base chat usage.
