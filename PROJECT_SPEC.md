# PROJECT_SPEC.md

## 1. Project Overview

**Project Name**

qt-llm

**Purpose**

qt-llm is a lightweight Qt/C++ toolkit for integrating Large Language Model services into Qt applications.

The project is intended to support:

- Qt desktop applications
- future Qt Creator integration
- local LLM services
- remote LLM services
- OpenAI-compatible APIs

The project should be friendly to both human developers and AI coding agents.

---

## 2. Design Goals

Primary goals:

1. Qt-native implementation
2. lightweight architecture
3. minimal external dependencies
4. provider-agnostic abstraction
5. local-first capability
6. async network communication
7. streaming token support
8. easy integration into existing Qt apps

---

## 3. Preferred Technical Direction

- Language: C++
- Framework: Qt
- Build system: qmake for the initial version
- IDE target: Qt Creator
- Main transport: HTTP-based APIs
- Baseline compatibility: OpenAI-compatible chat/completions style APIs

This project should avoid Python and avoid heavy frameworks unless a later phase justifies them.

---

## 4. Supported Provider Types

### Local providers

- Ollama
- vLLM
- llama.cpp-compatible servers
- LM Studio-compatible endpoints

### Remote providers

- OpenAI
- any OpenAI-compatible service

The internal design must not hard-code provider logic into the main client.

---

## 5. High-Level Architecture

```text
Qt Application
      │
      ▼
 QtLLMClient
      │
      ▼
 ILLMProvider
   ├── OpenAICompatibleProvider
   ├── OllamaProvider
   ├── VllmProvider
   └── Custom providers
```

Supporting modules:

- configuration
- request/response models
- network execution
- streaming parsing
- error handling

---

## 6. Core Components

### 6.1 QtLLMClient

Responsibilities:

- accepts prompts/messages from application code
- owns the active provider
- exposes async API via signals/slots
- exposes streaming callbacks/signals
- forwards provider/network errors

### 6.2 ILLMProvider

Responsibilities:

- define provider identity
- build request URL
- build request payload
- build headers
- parse response chunks
- parse final response

### 6.3 Network layer

Responsibilities:

- wrap QNetworkAccessManager usage
- keep all operations async
- provide a clean signal-based interface
- avoid blocking the UI thread

### 6.4 Streaming layer

Responsibilities:

- parse SSE-like or chunked response streams
- emit token fragments incrementally
- handle partial chunk buffering safely

### 6.5 Configuration model

Responsibilities:

- store base URL, API key, provider type, model name
- allow persistent loading/saving later
- remain simple in phase 1

---

## 7. Coding Principles

- prefer Qt-native types when appropriate
- use signals and slots for async flows
- use RAII
- avoid raw pointer ownership when possible
- keep interfaces small and composable
- avoid over-engineering
- keep provider implementations loosely coupled

---

## 8. Initial Scope (Phase 1)

Phase 1 should deliver:

- `QtLLMClient`
- `ILLMProvider`
- one OpenAI-compatible provider implementation
- one basic network wrapper
- one simple example UI
- repository documentation for AI tools and human contributors

---

## 9. Roadmap

### Phase 1

- OpenAI-compatible provider
- base models for request/response
- simple streaming support
- demo chat window

### Phase 2

- Ollama provider
- vLLM provider
- provider factory
- config persistence

### Phase 3

- richer prompt/message abstractions
- embeddings support
- tool-calling oriented abstractions
- better example applications

### Phase 4

- Qt Creator-oriented integration concepts
- plugin architecture discussion
- advanced provider capabilities

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
- depend on Python runtime
- embed heavyweight serving frameworks
- tightly couple UI code with provider internals

---

## 12. Instructions for AI coding agents

When continuing this project:

1. Read `PROJECT_SPEC.md`, `AI_RULES.md`, `ARCHITECTURE.md`, and `CODING_GUIDELINES.md` first.
2. Preserve provider abstraction.
3. Keep network operations asynchronous.
4. Prefer incremental, compilable changes.
5. Do not introduce unnecessary dependencies.
6. Use qmake unless explicitly asked to migrate to CMake.
7. Keep code readable for Qt developers using Qt Creator.
