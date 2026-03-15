# ROADMAP.md

## Phase 1: foundation
- qmake workspace setup
- core type definitions
- `QtLLMClient`
- `ILLMProvider`
- `OpenAICompatibleProvider`
- HTTP executor
- basic example app

## Phase 2: conversation and tool runtime
- conversation persistence
- tool registry and built-in tools
- tool selection and execution loop
- provider protocol adapters
- multi-client example UI

## Phase 3: MCP and observability
- MCP server registry and persistence
- MCP client bridge and tool sync
- MCP server manager demo and chat window
- unified runtime logging with file rotation and UI sink
- OpenAI `/responses` provider split

## Phase 4: hardening
- broader automated coverage for MCP and provider edge cases
- structured tool trace persistence
- richer log viewers and export helpers
- stronger policy and permission controls
