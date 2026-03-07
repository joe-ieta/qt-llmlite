# PROJECT_SPEC / 项目规格

## Summary / 摘要
This document defines the scope, goals, architecture direction, and phased delivery plan for qt-llmlite.

## 1. Project Overview
- Name: qt-llmlite (qt-llm)
- Purpose: provide a lightweight Qt/C++ toolkit for integrating LLM services into Qt applications
- Scope: local-first and OpenAI-compatible remote APIs

## 2. Design Goals
1. Qt-native implementation
2. Lightweight architecture
3. Minimal dependencies
4. Provider abstraction
5. Asynchronous networking
6. Streaming support
7. Easy integration

## 3. Technical Direction
- Language: C++
- Framework: Qt
- Build system: qmake (current baseline)
- IDE focus: Qt Creator
- Transport: HTTP APIs

## 4. Provider Types
- Local: Ollama, vLLM, llama.cpp-compatible, LM Studio-compatible
- Remote: OpenAI and OpenAI-compatible services

## 5. High-Level Architecture
`Qt Application -> QtLLMClient -> ILLMProvider -> HttpExecutor -> LLM service`

## 6. Core Components
- `QtLLMClient`: request orchestration, provider ownership, async signal API
- `ILLMProvider`: request/response shape, URL/headers/payload parsing
- `HttpExecutor`: async HTTP execution via `QNetworkAccessManager`
- Streaming parser: chunk/token incremental parsing
- Config model: base URL, API key, provider/model configuration

## 7. Coding Principles
- Keep interfaces small
- Keep provider logic decoupled
- Avoid blocking UI thread
- Prefer incremental, compilable changes

## 8. Initial Scope (Phase 1)
- `QtLLMClient`
- `ILLMProvider`
- OpenAI-compatible provider
- Basic HTTP executor
- Simple example app

## 9. Roadmap
- Phase 1: foundation
- Phase 2: provider expansion and config persistence
- Phase 3: richer abstractions and UX
- Phase 4: advanced AI integration and Qt Creator exploration

## 10. Constraints
- Must work on Windows and Linux
- Keep dependency/compile overhead low
- Support local/intranet model servers

## 11. Non-goals
- No model inference implementation
- No mandatory Python runtime for core C++ library
- No heavy serving frameworks embedded

## 12. Guidance for Coding Agents
- Read core docs first
- Preserve abstraction boundaries
- Keep networking async
- Prefer small, focused changes