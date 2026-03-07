# ARCHITECTURE / 架构说明

## Summary / 摘要
qt-llmlite is organized as a small Qt library plus examples, with strict dependency direction and asynchronous request flow.

## Dependency Flow
`Application/UI -> QtLLMClient -> ILLMProvider -> HttpExecutor -> LLM service`

## Principles
1. Provider abstraction first
2. Async-only networking
3. Streaming-ready interfaces
4. Small modules with clear responsibility

## Module Responsibilities
- `QtLLMClient`: orchestrates requests and public async API
- `ILLMProvider`: provider-specific URL/header/body/parse logic
- `HttpExecutor`: low-level HTTP request execution and signals
- Stream parser: incremental token parsing
- Example UI: integration reference only

## Runtime Flow
### Non-streaming
1. UI sends prompt
2. Provider builds request
3. Executor sends HTTP request
4. Provider parses final response
5. Client emits completion signal

### Streaming
1. UI sends prompt
2. Provider enables stream request
3. Network returns chunks
4. Stream parser accumulates chunks
5. Provider parses tokens
6. Client emits token and completion signals

## Future Evolution
- Provider factory
- Message/conversation model
- Embeddings layer
- Settings persistence
- Qt Creator-oriented integration