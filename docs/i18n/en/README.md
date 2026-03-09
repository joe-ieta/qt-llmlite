# README / 项目说明

## Summary / 摘要
qt-llmlite is a lightweight Qt/C++ LLM integration library that can be adopted as a reusable Qt package or integrated at source level.

## Project Introduction (updated after this merge)
This version evolves from a basic chat client into a production-oriented conversation and tool orchestration foundation:

- Multi-client factory with persistent restore by `clientId`
- One active session plus multiple historical sessions per client
- Profile-driven context injection (system prompt, capability, preference, style)
- Tool runtime abstraction (registry/selection/adapter/execution/policy/catalog)
- Built-in tool loop state machine in `QtLLMClient` with failure guard
- Vendor field mapping for OpenAI / Anthropic / Google
- Provider aliases for DeepSeek / Qwen / GLM families

## Integration Modes
- Standalone Qt package
- Source-level integration

## Main Documentation
- `PROJECT_SPEC.md`
- `ARCHITECTURE.md`
- `RELEASE_READINESS_CHECKLIST.md`
- `AI_RULES.md`
- `CODING_GUIDELINES.md`
- `DECISIONS.md`
