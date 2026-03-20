# Roadmap

## Phase 1: core baseline

- `QtLLMClient`
- provider abstraction
- HTTP executor
- base chat capability

## Phase 2: conversation and tools

- `ConversationClient` / `ConversationClientFactory`
- conversation persistence
- tool catalog, selection, execution, and failure guard

## Phase 3: MCP and observability

- MCP server registry, sync, and execution
- unified logging
- `toolsinside` trace and artifact recording

## Phase 4: asset and workspace management

- `toolstudio` workspaces
- import/export and merge
- editable tool metadata and placement organization

## Current Branch Focus

- clarify core-library boundaries
- improve consistency between provider, tool runtime, MCP, and `toolsinside`
- reduce documentation drift and naming ambiguity
