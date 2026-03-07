# DECISIONS / 设计决策

## Summary / 摘要
Key architectural decisions for qt-llmlite.

## Decision 1: qmake-first
The baseline uses qmake for smooth Qt Creator onboarding.

## Decision 2: OpenAI-compatible baseline
Provider abstraction targets widely adopted OpenAI-compatible HTTP formats.

## Decision 3: Local-first support
The project explicitly supports local/intranet model services (for example Ollama and vLLM).

## Decision 4: Lightweight architecture
Avoid heavy dependencies in the initial milestones.

## Decision 5: AI-agent-readable repository
Maintain structured context docs for both human and AI contributors.