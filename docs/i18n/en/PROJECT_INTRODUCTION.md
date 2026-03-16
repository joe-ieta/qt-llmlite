# PROJECT_INTRODUCTION / 项目简介

## Summary / 摘要
qt-llmlite is a lightweight Qt/C++ LLM integration library. The current baseline goes beyond basic chat and now includes multi-client persistence, a full tool runtime, MCP integration, and structured observability.

## Core Positioning
- Infrastructure layer for Qt LLM-enabled applications, not only a chat demo
- Provider abstraction with dedicated OpenAI and OpenAI-compatible paths
- Production-oriented extensibility for conversation and tool runtime

## Key Upgrades in This Merge
- Multi-client factory with persistent restore by `clientId`
- One active session plus multiple historical sessions (`clientId + sessionId`)
- Profile-driven context injection (system prompt, persona, preference, thinking style)
- Tool runtime abstraction (registry/selection/adapter/execution/policy/catalog)
- Built-in tool loop state machine in `QtLLMClient` with failure guard
- Dedicated `OpenAIProvider` for OpenAI `Responses API`
- `OpenAICompatibleProvider` path for Ollama and other `/chat/completions` style services
- Canonical tool registry with built-in tools plus MCP-imported tools
- MCP server registration, persistence, capability listing, tool sync, and execution routing
- Unified logging with per-client rotated JSONL files and Qt signal delivery
- Example apps: `simple_chat`, `multi_client_chat`, `mcp_server_manager_demo`

## Adoption Modes
- As a standalone Qt package
- As source-level integration in existing projects

## Key Value
- Lower integration barrier
- Reduce refactoring cost
- Extend tools and policies without breaking the base chat path
- Keep Windows/Linux consistency
- Improve diagnostics for provider, tool, and MCP runtime flows
