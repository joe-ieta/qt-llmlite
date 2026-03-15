# Testing Baseline / 测试基线

## Summary / 摘要
This document defines the current unit-test and smoke-test baseline for qt-llmlite.
本文件定义 qt-llmlite 当前的单元测试与冒烟测试基线。

## Scope / 覆盖范围
- `ProviderFactory` provider creation mapping
- `OpenAIProvider`
  - `/responses` URL/header build
  - payload JSON build
  - response parsing
  - stream delta parsing
- `OpenAICompatibleProvider`
  - `/chat/completions` URL/header build
  - payload JSON build
  - response parsing
  - stream token/delta parsing
- `StreamChunkParser`
  - fragmented input handling
  - pending line extraction
- tool loop and MCP paths currently rely on manual smoke testing

## Test Project / 测试工程
- Path: `tests/qtllm_tests/`
- Build file: `tests/qtllm_tests/qtllm_tests.pro`
- Framework: Qt Test (`QT += testlib`)

## Run (Qt Creator)
1. Open `qt-llm.pro`
2. Build `qtllm`
3. Build and run `qtllm_tests`

## Run (Command line, qmake)
```bash
qmake qt-llm.pro
make
./tests/qtllm_tests/qtllm_tests
```

Windows (MSVC example):
```bat
call E:\Qt\5.15.2\msvc2019_64\bin\qtenv2.bat
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
qmake qt-llm.pro -spec win32-msvc "CONFIG+=debug"
nmake /NOLOGO
.\tests\qtllm_tests\debug\qtllm_tests.exe
```

## Manual Smoke Baseline / 手工冒烟基线
- `simple_chat`: basic request and provider switch
- `multi_client_chat`: client/session switching and runtime log filtering
- `mcp_server_manager_demo`:
  - MCP server add/remove/persistence
  - MCP capability detail view (tools/resources/prompts)
  - MCP tool sync into shared registry
  - MCP-backed chat with selected tools/schema/request payload inspection

## Notes / 说明
- Network integration tests are not included in this baseline.
- OpenAI, Ollama, and MCP runtime behavior still require environment-backed smoke tests.
- This baseline focuses on deterministic tests for core abstractions and documented manual verification for external integrations.
