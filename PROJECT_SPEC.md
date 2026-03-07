# PROJECT_SPEC.md

## 1. Project Overview

**Project Name**

qt-llmlite (qt-llm)

**Purpose**

qt-llmlite is a lightweight Qt/C++ toolkit for integrating local-first Large Language Model services into Qt applications.

Primary support targets:

- Qt desktop applications
- local LLM services
- remote LLM services via OpenAI-compatible APIs

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
- OpenAI-compatible services

Provider logic must remain outside `QtLLMClient`.

---

## 5. High-Level Architecture

```text
Qt Application/UI
  -> QtLLMClient
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

---

## 6. Core Components

### 6.1 QtLLMClient

- receives prompts/messages from app code
- owns active provider
- exposes async API via signals/slots
- emits streaming tokens and completion/error events

### 6.2 ILLMProvider

- provider identity
- request URL/headers/payload generation
- final response parsing
- stream token parsing

### 6.3 HttpExecutor

- wraps `QNetworkAccessManager`
- emits `dataReceived`, `requestFinished`, `errorOccurred`
- keeps request flow async and UI-thread friendly

### 6.4 Streaming support

- incremental chunk handling
- token emission for stream mode
- safe handling of partial responses (current capability is basic)

### 6.5 Configuration model

- `providerName`, `baseUrl`, `apiKey`, `model`, `stream`

---

## 7. Current Implementation Status (as of 2026-03-07)

### Completed

- qmake multi-project workspace
- `QtLLMClient` with async request orchestration
- `ILLMProvider` abstraction
- `OpenAICompatibleProvider` request/response and stream-token parsing
- `HttpExecutor` async POST wrapper
- `OllamaProvider` and `VllmProvider` class stubs (name-level differentiation)
- `simple_chat` example with:
  - provider selection
  - base URL / API key input
  - model list fetching from `/models`
  - streaming output display
- cross-platform linking fixes for Windows (MSVC/MinGW) and Linux static library layout

### Not completed / partial

- dedicated provider-specific request/response adaptations for Ollama/vLLM beyond OpenAI-compatible baseline
- provider factory in core library
- robust SSE/stream parser integrated end-to-end for fragmented chunks
- config persistence
- automated tests (unit/integration)
- packaging/distribution workflow for reusable Qt package artifacts

---

## 8. Scope Baseline (Current)

Current baseline includes:

- reusable core library (`src/qtllm`)
- one working OpenAI-compatible provider path
- demo Qt Widgets application (`simple_chat`)
- documentation set (English + Chinese)

---

## 9. Roadmap (Planning)

### Phase 2: provider completion and platform stability

- provider factory in core
- stronger Ollama/vLLM compatibility behavior
- robust streaming parser integration
- request timeout/retry/cancellation support

### Phase 3: developer usability

- richer message abstractions (system/user/assistant history)
- optional config persistence
- improved example UX and validation
- test suite foundation

### Phase 4: advanced capabilities

- embeddings abstraction
- tool-calling oriented abstractions
- retrieval-oriented helper APIs

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