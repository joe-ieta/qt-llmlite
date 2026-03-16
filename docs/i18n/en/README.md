# README / Project Notes

## Summary
qt-llmlite is a lightweight Qt/C++ LLM integration library that can be adopted as a reusable Qt package or integrated at source level.

## Project Introduction
This version evolves from a basic chat client into a production-oriented conversation and tool orchestration foundation:

- Multi-client factory with persistent restore by `clientId`
- One active session plus multiple historical sessions per client
- Profile-driven context injection (system prompt, capability, preference, style)
- Tool runtime abstraction (registry/selection/adapter/execution/policy/catalog)
- Built-in tool loop state machine in `QtLLMClient` with failure guard
- Dedicated `OpenAIProvider` for OpenAI `Responses API`
- `OpenAICompatibleProvider` path for Ollama and other `/chat/completions` style services
- MCP server registration, persistence, capability inspection, tool sync, and execution routing
- Unified logging with per-client rotated JSONL files and Qt UI signal delivery
- Example apps: `simple_chat`, `multi_client_chat`, `mcp_server_manager_demo`

## Integration Modes
- Standalone Qt package
- Source-level integration

## Main Documentation
- `PROJECT_SPEC.md`
- `ARCHITECTURE.md`
- `RELEASE_READINESS_CHECKLIST.md`
- `AI_RULES.md`
- `CODING_GUIDELINES.md`
