# qt-llm

A lightweight, Qt-native toolkit for integrating LLM services into Qt applications.

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
- `docs`: roadmap and architecture decisions
- `tools/qtcreator`: Qt Creator External Tool templates
- `src/qt_llm`: optional Python CLI bridge for Qt Creator tooling

## Build (qmake)

### Open in Qt Creator

1. Open `qt-llm.pro`
2. Select your Qt kit
3. Build `qtllm` and `simple_chat`

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
  scripts/
  src/
    examples/simple_chat/
    qtllm/
    qt_llm/
  tools/qtcreator/
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