# PROJECT_SPEC.md

## 1. Project Overview

**Project Name**

qt-llmlite (qt-llm)

**Purpose**

qt-llmlite is a lightweight Qt/C++ toolkit for integrating local-first Large Language Model services into Qt applications.

Primary support targets:

- Qt desktop applications
- local LLM services
- remote LLM services via OpenAI-compatible APIs and vendor APIs

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

---

## 3. Technical Direction

- Language: C++
- Framework: Qt
- Build system: qmake
- IDE target: Qt Creator
- Main transport: HTTP-based APIs
- Baseline API shape: OpenAI-compatible `/chat/completions`

Constraints:

- keep core library Python-free
- avoid heavy frameworks/dependencies

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
- DeepSeek / Qwen / GLM class providers via mapped schema path

Provider logic remains outside application UI and is encapsulated by `ILLMProvider` implementations.

---

## 5. High-Level Architecture

```text
Qt Application/UI
  -> (A) QtLLMClient                        [base + built-in tool loop capable]
  -> (B) ToolEnabledChatEntry               [optional orchestration entry]
        -> tool selection + adapter + policy
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
- error propagation
- conversation client factory and persistence
- tool registry / catalog / policy / execution / protocol routing

---

## 6. Core Components

### 6.1 QtLLMClient

- receives prompts/messages from app code
- owns active provider and executor
- exposes async API via signals/slots
- emits streaming tokens and completion/error events
- supports structured response event (`responseReceived`)
- includes built-in tool-loop state handling with failure guard

### 6.2 ConversationClient + ConversationClientFactory

- creates and manages multiple client identities (`clientId`)
- persists and restores client conversation state
- for each client, tracks one active session and multiple historical sessions
- supports session switching for history viewing (`clientId + sessionId`)
- binds tool-loop context (`clientId`, `sessionId`) into request path

### 6.3 ILLMProvider

- provider identity
- request URL/headers/payload generation
- final response parsing
- stream token parsing
- capability signal for structured tool calls

### 6.4 HttpExecutor

- wraps `QNetworkAccessManager`
- emits `dataReceived`, `requestFinished`, `errorOccurred`
- keeps request flow async and UI-thread friendly

### 6.5 Tool/function-calling abstraction

- `LlmToolRegistry` maintains tool list (`llmTools`)
- canonical internal tool schema (`LlmToolDefinition`)
- built-in non-removable tools (`current_time`, `current_weather`)
- `ToolSelectionLayer` chooses tools per turn
- `ToolCallProtocolRouter` routes by model vendor first
- `ToolExecutionLayer` + executor registry handles tool execution abstraction
- `ToolCallOrchestrator` handles parsing/execution/follow-up/failure-guard loop
- `ToolEnabledChatEntry` is optional app entry and wiring point

### 6.6 Tool persistence and policy

- `ToolCatalogRepository` persists tool catalog
- `ClientToolPolicyRepository` persists per-client tool policies
- built-ins are auto-injected at startup and protected from deletion

### 6.7 Configuration model

- `providerName`, `baseUrl`, `apiKey`, `model`, `modelVendor`, `stream`

---

## 7. Current Implementation Status (as of 2026-03-08)

### Completed

- qmake multi-project workspace
- `QtLLMClient` with async request orchestration and built-in tool-loop support
- `ILLMProvider` abstraction
- `OpenAICompatibleProvider`:
  - OpenAI-style request/response + stream parsing
  - Anthropic request/response field mapping (`/messages`, `tool_use` blocks)
  - Google request/response field mapping (`generateContent`, `functionCall` parts)
- provider creation aliases for OpenAI/Anthropic/Google/DeepSeek/Qwen/GLM classes
- `HttpExecutor` async POST wrapper
- provider implementations for Ollama / vLLM / OpenAI-compatible paths
- conversation factory architecture with persistence:
  - multiple clients
  - per-client active session
  - per-client historical sessions
- tools runtime stack:
  - registry, built-ins, protocol adapters/router
  - execution layer, orchestrator, catalog/policy repositories
- `simple_chat` example retained
- `multi_client_chat` example showcasing multi-client + session history + tools entry
- baseline tests extended for provider field mappings and aliases

### Partial / pending hardening

- stricter provider-native schema coverage for more vendors
- stronger streaming edge-case handling across all vendors
- richer tool trace persistence in session history
- broader automated tests for tool-loop state machine and persistence flows
- packaging/distribution workflow for reusable Qt package artifacts

---

## 8. Scope Baseline (Current)

Current baseline includes:

- reusable core library (`src/qtllm`)
- multi-client/session persistence foundation
- vendor-mapped provider path in current provider stack
- tools runtime abstraction with built-in tool defaults
- demo Qt Widgets applications (`simple_chat`, `multi_client_chat`)
- documentation set (English + Chinese)

---

## 9. Roadmap (Next)

### Phase 2: protocol and runtime hardening

- expand vendor-native protocol adapters in provider layer
- robust stream parser behavior for fragmented payloads
- timeout/retry/cancel hardening under tool-loop chains

### Phase 3: conversation and trace quality

- persist structured tool-call/tool-result traces
- improve session replay/debuggability
- strengthen test coverage for factory/session/tool-loop interactions

### Phase 4: production controls

- permission and safety policies per tool category
- observability fields and audit trails
- model/tool routing optimization

---

## 10. Constraints

- must work on Windows and Linux
- should stay friendly to Qt Creator users
- should support local intranet-hosted LLM servers
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
7. Keep tool/runtime changes additive and backward compatible for base chat usage.
