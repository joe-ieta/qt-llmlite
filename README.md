# qt-llmlite (qt-llm)

## 简介 / Introduction

### 中文
`qt-llmlite` 是一个面向 Qt 开发者的轻量级本地模型接入库。项目基于 Qt/C++ 提供统一客户端与 Provider 抽象，可快速连接 Ollama、vLLM 及其他 OpenAI 兼容接口。它既可作为独立 Qt 包引入，也可按源码级方式集成到现有工程，以较低改造成本完成本地大模型能力接入。

### English
`qt-llmlite` is a lightweight local-LLM integration library for Qt developers. It provides a Qt/C++ native client with a provider abstraction to quickly connect to Ollama, vLLM, and other OpenAI-compatible endpoints. The project can be used either as a standalone Qt package or integrated directly at source level into existing applications, enabling fast local model adoption with minimal refactoring cost.

## Language Docs
- English docs: `docs/i18n/en/`
- 中文文档：`docs/i18n/zh-CN/`
- About snippets: `docs/ABOUT_SNIPPETS.md`

## Tests
- Unit-test baseline: `tests/qtllm_tests` (Qt Test)
- Testing guide: `docs/TESTING_BASELINE.md`

## Goals
- Qt-native C++ implementation
- Support local and remote LLM services
- Prefer OpenAI-compatible HTTP APIs
- Keep dependencies minimal
- Work well in Qt Creator
- Remain lightweight and extensible

## What is in this repository
- `src/qtllm`: C++ Qt library core (`QtLLMClient`, providers, networking, streaming parser)
- `src/examples/simple_chat`: minimal Qt Widgets chat example
- `tests/qtllm_tests`: Qt Test unit baseline
- `docs`: roadmap, decisions, and project introduction

## Build (qmake)

### Open in Qt Creator
1. Open `qt-llm.pro`
2. Select your Qt kit
3. Build `qtllm`, `qtllm_tests`, and `simple_chat`

### Command line
```bash
qmake qt-llm.pro
make
```

Windows + MinGW:
```bash
qmake qt-llm.pro
mingw32-make
```

## Repository layout
```text
qt-llm/
  .github/
  docs/
  src/
    examples/simple_chat/
    qtllm/
  tests/
    qtllm_tests/
  qt-llm.pro
```

## Notes for contributors and coding agents
Read in this order before development:
1. `PROJECT_SPEC.md`
2. `AI_RULES.md`
3. `ARCHITECTURE.md`
4. `CODING_GUIDELINES.md`
5. `docs/DECISIONS.md`

## License
MIT. See `LICENSE`.