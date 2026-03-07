# PROJECT_SPEC / 项目规格

## 摘要 / Summary
This document defines scope, architecture direction, and current implementation status for qt-llmlite.

## 1. Project Overview
- Name: qt-llmlite (qt-llm)
- Purpose: lightweight Qt/C++ local-first LLM integration toolkit
- Targets: Qt desktop apps, local providers, OpenAI-compatible remote APIs

## 2. Goals
1. Qt-native implementation
2. Lightweight architecture
3. Minimal dependencies
4. Provider abstraction
5. Async networking
6. Streaming support
7. Easy integration

## 3. Technical Direction
- Language: C++
- Framework: Qt
- Build: qmake
- IDE: Qt Creator
- Transport: HTTP
- Baseline API: OpenAI-compatible `/chat/completions`

## 4. Architecture
`Qt Application -> QtLLMClient -> ILLMProvider -> HttpExecutor -> LLM service`

## 5. Current Status (2026-03-07)
### Completed
- qmake workspace
- `QtLLMClient`
- `ILLMProvider`
- `OpenAICompatibleProvider`
- async `HttpExecutor`
- `OllamaProvider`/`VllmProvider` class stubs
- `simple_chat` with provider switch, model list fetch, streaming UI
- Windows/Linux linking stabilization

### Partial / Pending
- provider-specific Ollama/vLLM adaptations
- provider factory
- robust fragmented-stream parser flow
- config persistence
- test suite
- packaging workflow

## 6. Planned Phases
### Phase 2
- provider factory
- improved provider compatibility
- streaming robustness
- timeout/retry/cancel

### Phase 3
- richer message abstraction
- optional persistence
- better example UX
- tests

### Phase 4
- embeddings
- tool-calling abstractions
- retrieval helper APIs

## 7. Constraints
- Windows + Linux support
- local/intranet friendly
- low dependency overhead

## 8. Non-goals
- no model inference implementation
- no heavy serving framework embedding
- no UI/provider tight coupling