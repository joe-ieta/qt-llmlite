# DECISIONS.md

## Decision 1: qmake first

The initial repository uses qmake because the intended user workflow is strongly aligned with Qt Creator and a straightforward starter experience.

## Decision 2: OpenAI-compatible baseline

Provider abstraction is designed around OpenAI-compatible HTTP APIs because many local and remote providers now expose similar request/response shapes.

## Decision 3: local-first friendly

The project must support intranet or local model servers such as Ollama and vLLM, not just public cloud APIs.

## Decision 4: lightweight architecture

Avoid heavy runtime dependencies in the initial version. The first milestone is a clean Qt-native skeleton with a clear extension path.

## Decision 5: AI-agent-readable repository

This repository intentionally includes multiple context markdown files so coding agents can continue work with minimal repeated explanation.
