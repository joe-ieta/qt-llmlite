# Delivery and Integration / 交付与集成说明

## Summary / 摘要
This document defines deliverables and integration paths for qt-llmlite.
本文件定义 qt-llmlite 的交付物与集成路径。

## 1. Deliverables / 交付物

### 1.1 Core Library
- Source path: `src/qtllm/`
- Build output: static library (`qtllm.lib` on MSVC, `libqtllm.a` on MinGW/Linux)
- Public capability:
  - `QtLLMClient`
  - provider abstraction (`ILLMProvider`)
  - provider factory (`ProviderFactory`)
  - async HTTP execution with timeout/retry/cancel
  - streaming token and stream-delta handling
  - canonical tool registry and tool loop
  - MCP server registry and tool sync API
  - structured logging API

### 1.2 Example Applications
- `src/examples/simple_chat/`: basic provider integration reference
- `src/examples/multi_client_chat/`: multi-client, session history, runtime log view
- `src/examples/mcp_server_manager_demo/`: MCP server management and MCP-backed chat demo

### 1.3 Documentation
- Architecture, spec, roadmap, release checklist, testing baseline
- English docs: `docs/i18n/en/`
- Chinese docs: `docs/i18n/zh-CN/`

## 2. Integration Modes / 集成方式

### Mode A: Source-level integration
1. Copy `src/qtllm/` into your repository.
2. Add `qtllm.pro` as a subproject or include sources directly.
3. Link against Qt modules: `core`, `network`, and `widgets` when using example-style UI helpers.
4. Initialize `QtLLMClient` or `ConversationClient`, set config/provider, then send requests asynchronously.

### Mode B: Library artifact integration
1. Build `qtllm` static library in CI or local environment.
2. Export public headers from `src/qtllm/`.
3. Consume library + headers in the business project.
4. Keep ABI/compiler settings aligned with the consumer project.

## 3. Minimal Integration Contract / 最小集成契约
- Required Qt modules for core library: `QT += core network`
- Add `QT += widgets` for the example-style UI surfaces
- C++ standard: C++17
- Build system baseline: qmake
- Runtime contract:
  - provide `baseUrl`, `model`, and `providerName`
  - provide `apiKey` when the selected provider requires authorization

## 4. Provider Selection Contract / Provider 选择约定
- `openai` -> `OpenAIProvider`
- `openai-compatible` -> `OpenAICompatibleProvider`
- `ollama` -> `OllamaProvider`
- `vllm` -> `VllmProvider`
- `sglang`, `anthropic`, `google`, `gemini`, `deepseek`, `qwen`, `glm`, `zhipu` -> `OpenAICompatibleProvider`

## 5. Request and Tool Contract / 请求与工具约定
- `OpenAIProvider` targets `/responses`
- `OpenAICompatibleProvider` targets `/chat/completions`
- Canonical tool identity is split into:
  - `toolId`: internal routing and persistence
  - `invocationName`: provider-visible function name
  - `name`: display name
- MCP-imported tools are registered into the same shared tool registry as built-in tools

## 6. Logging Contract / 日志约定
- Logger entry point: `QtLlmLogger`
- File sink: `FileLogSink`
- UI sink: `SignalLogSink`
- Default storage location: workspace `.qtllm/logs/<clientId>/`
- Encoding: UTF-8 JSONL
- Rotation: size-based, pruned by per-client file-count limit

## 7. Recommended Release Checklist / 推荐发布检查项
1. Build on Windows and Linux.
2. Validate non-streaming and streaming calls.
3. Validate timeout/retry/cancel behavior.
4. Verify provider selection and provider-specific endpoint path.
5. Verify MCP server persistence and tool sync.
6. Verify per-client log rotation and UI log delivery.
7. Update `PROJECT_SPEC.md`, `ARCHITECTURE.md`, and release checklist when scope changes.
