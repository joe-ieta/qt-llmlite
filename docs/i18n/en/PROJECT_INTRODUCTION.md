# PROJECT_INTRODUCTION / 项目简介

## Summary / 摘要
qt-llmlite is a lightweight Qt/C++ LLM integration library. The current baseline goes beyond basic chat and includes multi-client conversation persistence and tool runtime orchestration.

## Core Positioning
- Infrastructure layer for Qt LLM-enabled applications, not only a chat demo
- Provider abstraction with vendor protocol field mapping
- Production-oriented extensibility for conversation and tool runtime

## Key Upgrades in This Merge
- Multi-client factory with persistent restore by `clientId`
- One active session plus multiple historical sessions (`clientId + sessionId`)
- Profile-driven context injection (system prompt, persona, preference, thinking style)
- Tool runtime abstraction (registry/selection/adapter/execution/policy/catalog)
- Built-in tool loop state machine in `QtLLMClient` with failure guard
- Fixed built-in tools: `current_time`, `current_weather` (non-removable)
- Provider field mapping: OpenAI / Anthropic / Google
- Provider aliases for DeepSeek / Qwen / GLM families

## Adoption Modes
- As a standalone Qt package
- As source-level integration in existing projects

## Key Value
- Lower integration barrier
- Reduce refactoring cost
- Extend tools/policies without breaking the base chat path
- Keep Windows/Linux consistency
