# ARCHITECTURE.md

## Overview

qt-llm is structured as a small Qt library plus example applications.

The core dependency flow should remain:

```text
Application/UI
    ↓
QtLLMClient
    ↓
ILLMProvider
    ↓
HttpExecutor / Qt network layer
    ↓
Remote or local LLM service
```

## Architectural principles

### 1. Provider abstraction first

The client should not know provider-specific JSON formats beyond what is delegated through the provider interface.

### 2. Async-only networking

All requests must be non-blocking and signal-driven.

### 3. Streaming-ready design

Even if early implementations are basic, interfaces should anticipate streamed output.

### 4. Small modules

Each module should have a narrow concern:

- client orchestration
- provider behavior
- HTTP execution
- stream parsing
- example UI

## Suggested source layout

```text
src/
  qtllm/
    core/
      qtllmclient.h/.cpp
      llmtypes.h
      llmconfig.h
    providers/
      illmprovider.h
      openaicompatibleprovider.h/.cpp
      ollamaprovider.h/.cpp
      vllmprovider.h/.cpp
    network/
      httpexecutor.h/.cpp
    streaming/
      streamchunkparser.h/.cpp
  examples/
    simple_chat/
      main.cpp
      chatwindow.h/.cpp
```

## Runtime flow

### Non-streaming flow

1. UI calls `QtLLMClient::sendPrompt(...)`
2. Client asks active provider to build request
3. Client passes request to `HttpExecutor`
4. Network reply returns raw bytes
5. Provider parses final response
6. Client emits completion signal

### Streaming flow

1. UI calls `QtLLMClient::sendPrompt(...)`
2. Provider builds stream-enabled request
3. Network reply produces incremental data
4. Stream parser accumulates chunks
5. Provider parses chunk/token fragments
6. Client emits `tokenReceived(...)`
7. Client emits final `finished(...)`

## Design boundaries

### `QtLLMClient`

Should manage:

- active provider
- network executor
- lifecycle of a request
- public API exposed to Qt app code

Should not manage:

- provider-specific JSON details
- concrete widget rendering

### `ILLMProvider`

Should manage:

- URLs
- headers
- JSON bodies
- parsing strategy

Should not manage:

- UI behavior
- top-level request scheduling beyond its own payload logic

### `HttpExecutor`

Should manage:

- `QNetworkAccessManager`
- HTTP method execution
- low-level reply wiring

Should not manage:

- provider-specific parsing rules

## Future evolution

Likely future additions:

- provider factory
- conversation/message model
- embeddings API layer
- settings persistence service
- Qt Creator-specific integration layer

Any major addition should preserve the current dependency direction and avoid circular coupling.
