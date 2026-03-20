# Project Introduction

`qt-llm` is a Qt/C++ LLM integration project centered on the `qtllm` static library. It is no longer only a chat demo repository; it now provides a layered foundation for:

- base request and streaming response handling
- multi-client and multi-session conversation management
- tool calling and follow-up loops
- MCP server integration
- structured logging and trace analysis
- tool catalog and workspace management

The repository currently contains one core library plus five application entry points, each serving as a host for a different subsystem of the stack.
