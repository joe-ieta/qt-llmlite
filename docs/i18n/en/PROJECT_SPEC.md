# PROJECT_SPEC / Project Spec

## Summary
This document defines the current scope, architecture direction, and implementation status for qt-llm.

## 1. Overview
- Name: qt-llmlite (qt-llm)
- Purpose: lightweight Qt/C++ toolkit for local-first and remote LLM integration
- Targets: Qt desktop apps, local providers, vendor APIs, OpenAI-compatible services

## 2. Goals
1. Qt-native implementation
2. lightweight architecture
3. minimal dependencies
4. provider abstraction
5. async networking
6. streaming support
7. easy integration
8. multi-client persistence
9. tool/function-calling support
10. MCP integration
11. structured observability

## 3. Technical Direction
- Language: C++17
- Framework: Qt
- Build: qmake
- IDE: Qt Creator
- Transport: HTTP
- OpenAI path: `/responses`
- OpenAI-compatible path: `/chat/completions`

## 4. Core Architecture
`Qt Application -> ToolEnabledChatEntry or ConversationClient -> QtLLMClient -> ILLMProvider -> HttpExecutor -> LLM service`

## 5. Current Status (2026-03-15)
### Completed
- dedicated `OpenAIProvider`
- `OpenAICompatibleProvider`
- `OllamaProvider` / `VllmProvider`
- multi-client/session persistence
- internal tool loop
- canonical tool registry with built-ins
- MCP server registry, persistence, sync, and execution routing
- unified logging with file sink and UI sink
- example apps: `simple_chat`, `multi_client_chat`, `mcp_server_manager_demo`

### Pending hardening
- more automated tests for MCP and OpenAI Responses API edge cases
- structured tool trace persistence
- stronger external-tool async execution model

## 6. Canonical Tool Model
- `toolId`: internal unique identifier
- `invocationName`: provider-visible function name
- `name`: display label

## 7. Constraints
- Windows and Linux support
- Qt Creator friendly workflow
- low dependency overhead
- local/intranet friendly deployment

## 8. Non-goals
- no model inference implementation
- no heavyweight serving framework embedding
- no tight UI/provider coupling
