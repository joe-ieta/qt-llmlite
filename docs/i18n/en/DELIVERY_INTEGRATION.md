# DELIVERY_INTEGRATION

## Summary
This document defines the deliverables and integration paths for qt-llmlite.

## 1. Deliverables

### 1.1 Core Library
- Source: `src/qtllm/`
- Build output: static library (`qtllm.lib` on MSVC, `libqtllm.a` on MinGW/Linux)
- Public capabilities:
  - `QtLLMClient`
  - `ILLMProvider`
  - `ProviderFactory`
  - async HTTP execution with timeout/retry/cancel
  - streaming token handling

### 1.2 Example Application
- Source: `src/examples/simple_chat/`
- Purpose: Qt Widgets integration reference

### 1.3 Documentation
- Specs, architecture, roadmap, decisions
- English docs: `docs/i18n/en/`
- Chinese docs: `docs/i18n/zh-CN/`

## 2. Integration Modes

### Mode A: Source-level integration
1. Copy `src/qtllm/` into your repository.
2. Add `qtllm.pro` as subproject or include source files directly.
3. Link required Qt modules: `core`, `network`.
4. Configure `QtLLMClient` and send requests asynchronously.

### Mode B: Package-style integration
1. Build the static library artifact.
2. Export/install public headers from `src/qtllm/`.
3. Link library + headers in consumer project.
4. Keep compiler/ABI settings consistent.

## 3. Minimal Contract
- `QT += core network`
- C++17
- qmake baseline
- Required runtime config: `providerName`, `baseUrl`, `model` (+ `apiKey` when required)

## 4. Provider Mapping
- `openai-compatible` -> `OpenAICompatibleProvider`
- `openai` -> `OpenAICompatibleProvider`
- `ollama` -> `OllamaProvider`
- `vllm` -> `VllmProvider`
- `sglang` -> `OpenAICompatibleProvider`

## 5. Request Control
`LlmConfig` fields:
- `timeoutMs`
- `maxRetries`
- `retryDelayMs`

`QtLLMClient` API:
- `cancelCurrentRequest()`

## 6. Release Checklist
1. Build on Windows and Linux.
2. Verify streaming and non-streaming paths.
3. Verify timeout/retry/cancel behavior.
4. Verify provider mapping and invocation path.
5. Sync specs/roadmap when scope changes.