# qt-llmlite Project Introduction (English)

## Positioning

`qt-llmlite` is a lightweight Qt/C++ infrastructure library for integrating LLM capabilities into applications.

It is no longer only a chat demo baseline. The current foundation includes:

- Multi-client factory with persistence (`clientId` restore)
- One active session plus multiple historical sessions per client
- Profile-driven context injection (system prompt / persona / preference / thinking style)
- Tool runtime abstraction (registry / selection / adapter / execution / policy / catalog)
- Internal tool-loop state machine in `QtLLMClient` with failure guard
- Dedicated `OpenAIProvider` for OpenAI `Responses API`
- `OpenAICompatibleProvider` and Ollama path for `/chat/completions` style services
- MCP server registration, persistence, tool sync, capability inspection, and MCP-backed tool execution
- Structured logging with per-client rotated JSONL files and live Qt signal delivery
- Example applications: `simple_chat`, `multi_client_chat`, `mcp_server_manager_demo`

## Core Value

- Low-intrusion integration into existing Qt projects
- Clear upgrade path from basic chat to tool orchestration
- Stable provider abstraction boundaries for future expansion
- Observable runtime behavior for debugging provider, tool, and MCP flows

## Entry Documents

- [README](./README.md)
- [Project Spec](./PROJECT_SPEC.md)
- [Architecture](./ARCHITECTURE.md)
- [Release Readiness Checklist](./RELEASE_READINESS_CHECKLIST.md)
