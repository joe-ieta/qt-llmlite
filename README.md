# qt-llmlite (qt-llm)

## Navigation

- [中文介绍](./INTRODUCTION.zh-CN.md)
- [English Introduction](./INTRODUCTION.en.md)
- [项目规格 / Project Spec](./PROJECT_SPEC.md)
- [架构说明 / Architecture](./ARCHITECTURE.md)
- [发布前检查 / Release Checklist](./RELEASE_READINESS_CHECKLIST.md)
- [交付与集成 / Delivery Integration](./docs/DELIVERY_INTEGRATION.md)
- [测试基线 / Testing Baseline](./docs/TESTING_BASELINE.md)

## Project Snapshot

`qt-llmlite` is a lightweight Qt/C++ LLM integration library for local-first and remote model access.

Current baseline includes:
- multi-client and multi-session conversation persistence
- provider abstraction with dedicated `OpenAIProvider` and `OpenAICompatibleProvider`
- internal tool-call loop in `QtLLMClient`
- canonical tool registry with built-in tools and MCP-imported external tools
- MCP server registration, persistence, tool sync, and execution routing
- structured runtime logging with file rotation and UI signal delivery

## Examples

- `src/examples/simple_chat`
- `src/examples/multi_client_chat`
- `src/examples/mcp_server_manager_demo`

## Runtime Workspace Data

Runtime state is written under the workspace-local `.qtllm/` directory, including:
- conversation and tool persistence
- MCP server registry data
- per-client rotated JSONL logs

## Docs

- Chinese docs: `docs/i18n/zh-CN/`
- English docs: `docs/i18n/en/`

## License

MIT. See `LICENSE`.
