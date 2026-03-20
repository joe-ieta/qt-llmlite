# Repository Structure

## Top Level

- `README.md`
  - project overview, reading order, and documentation governance
- `PROJECT_SPEC.md`
  - current scope, goals, and non-goals
- `ARCHITECTURE.md`
  - layering, module responsibilities, and runtime flows
- `qt-llm.pro`
  - qmake workspace entry

## Core Source

- `src/qtllm/`
  - `core/`: `QtLLMClient`, config, and base types
  - `chat/`: conversation clients, factory, and snapshots
  - `profile/`: profile and memory policy
  - `storage/`: conversation persistence
  - `providers/`: provider abstraction and implementations
  - `network/`: HTTP execution layer
  - `streaming/`: stream chunk parsing
  - `tools/`: tool registry, selection, protocol, execution, and MCP
  - `logging/`: unified logging entry points and sinks
  - `toolsinside/`: trace, artifact, query, and admin services
  - `toolsstudio/`: tool catalog, workspaces, and import/export services

## Applications

- `src/apps/simple_chat/`
  - minimal chat integration surface
- `src/apps/multi_client_chat/`
  - multi-client/session and logging integration
- `src/apps/mcp_server_manager/`
  - MCP management and MCP-backed chat
- `src/apps/tools_inside/`
  - QML trace browser
- `src/apps/toolstudio/`
  - tool catalog and workspace management UI

## Tests

- `tests/qtllm_tests/`
  - Qt Test baseline for core library behavior

## Documentation

- `docs/PROJECT_INTRODUCTION.md`
- `docs/DELIVERY_INTEGRATION.md`
- `docs/TESTING_BASELINE.md`
- `docs/ROADMAP.md`
- `docs/DECISIONS.md`
- `docs/releases/`
- `docs/i18n/`
  - retained as translation mirrors, no longer the primary entry point

## Runtime Workspace Layout

The project writes runtime state under `.qtllm/`:
- `.qtllm/clients/`
- `.qtllm/mcp/`
- `.qtllm/tools/`
- `.qtllm/logs/`
- `.qtllm/tools_inside/`
