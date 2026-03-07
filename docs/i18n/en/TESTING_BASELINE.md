# TESTING_BASELINE

## Summary
Current unit-test baseline for qt-llmlite.

## Coverage
- `ProviderFactory` mapping
- `OpenAICompatibleProvider` build/parse behavior
- `StreamChunkParser` fragmented-stream behavior

## Project
- `tests/qtllm_tests/`
- Qt Test based (`QT += testlib`)

## Run
1. Build `qtllm`
2. Build/run `qtllm_tests`

CLI:
```bash
qmake qt-llm.pro
make
./tests/qtllm_tests/qtllm_tests
```