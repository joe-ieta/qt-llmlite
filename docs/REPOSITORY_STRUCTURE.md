# Repository Structure

## Top-level

- `README.md`: project overview and quick start
- `PROJECT_SPEC.md`: scope, goals, and phased plan
- `ARCHITECTURE.md`: dependency flow and module boundaries
- `AI_RULES.md`: coding-agent rules
- `CODING_GUIDELINES.md`: code conventions
- `qt-llm.pro`: qmake workspace entry

## Source code

- `src/qtllm/`: core C++ library
  - `core/`: client and shared types
  - `providers/`: provider abstraction and implementations
  - `network/`: HTTP execution layer
  - `streaming/`: chunk/token parser
- `src/examples/simple_chat/`: minimal Qt Widgets integration example
- `src/qt_llm/`: optional Python CLI bridge for Qt Creator external tools

## Supporting assets

- `docs/`: roadmap and decisions
- `scripts/`: helper setup scripts
- `tools/qtcreator/`: Qt Creator External Tool templates
- `.github/`: issue and PR templates