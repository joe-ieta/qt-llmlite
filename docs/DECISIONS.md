# Decisions

## Decision 1: keep `qtllm` as the center

The repository can contain multiple application entry points, but `src/qtllm/` remains the architectural center. Applications host and validate subsystems; they do not redefine the library boundaries.

## Decision 2: provider abstraction stays first-class

Vendor-specific protocol details should converge into `ILLMProvider` implementations and related adapters instead of leaking into UI and orchestration layers.

## Decision 3: tool runtime is baseline architecture

Tool calling is no longer an optional add-on. Tool registry, protocol routing, execution, and failure guard are part of the main architecture.

## Decision 4: MCP integrates through the shared tool model

MCP tools are not a separate runtime universe. They enter through the shared `LlmToolRegistry` and are routed later by canonical `toolId`.

## Decision 5: observability is split into two systems

- `logging`: operational troubleshooting and live viewing
- `toolsinside`: trace-grade structured analysis

Both remain, but they serve different purposes.

## Decision 6: reduce duplicate documentation

Root documents and the main files under `docs/` are the primary documentation entry points. Repeated, obsolete, or unclear-purpose documents should be merged or removed.
