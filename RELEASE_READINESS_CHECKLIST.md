# RELEASE_READINESS_CHECKLIST.md

## Scope

This checklist targets the current qt-llm baseline after:

- multi-client factory + session persistence
- tool runtime abstraction + built-in tools
- QtLLMClient internal tool-loop state machine
- vendor-mapped provider protocol support

## 1. Build and Packaging

- [ ] `nmake` completes successfully for library, tests, and examples.
- [ ] `simple_chat` starts and sends a basic non-stream request.
- [ ] `multi_client_chat` starts and basic UI interactions are stable.
- [ ] No new mandatory external dependency was introduced.

## 2. Core Chat Regression

- [ ] Base chat path works without tool entry (`ConversationClient -> QtLLMClient`).
- [ ] Streaming tokens arrive in order and final completion is emitted once.
- [ ] Error path emits `errorOccurred` and does not deadlock further requests.

## 3. Multi-Client / Session Persistence

- [ ] New client creation generates unique `clientId`.
- [ ] Re-acquiring an existing `clientId` restores previous config/profile/history.
- [ ] Per client, only one active session is used at a time.
- [ ] Finished dialogue can be archived by creating a new session.
- [ ] Session switching displays the correct historical messages.

## 4. Tool Catalog and Policy Persistence

- [ ] Tool catalog file is created under `.qtllm/tools/catalog.json`.
- [ ] Built-in tools (`current_time`, `current_weather`) are auto-injected on startup.
- [ ] Built-in tools are marked non-removable and enforced as enabled.
- [ ] Client tool policy file persists under `.qtllm/clients/<clientId>/tool_policy.json`.

## 5. Tool Runtime Behavior

- [ ] Tool selection layer can filter tools by profile/history.
- [ ] Execution layer enforces allow/deny policy and per-turn limits.
- [ ] Default dry-run mode returns structured failure without crash.
- [ ] Internal loop stops after failure guard threshold (no infinite loop).
- [ ] Loop context uses `clientId + sessionId` correctly.

## 6. Provider Protocol Mapping

- [ ] OpenAI path builds `/chat/completions` and parses tool_calls.
- [ ] Anthropic mapping builds `/messages` with expected headers and tool blocks.
- [ ] Google mapping builds `...:generateContent` with expected function-call parts.
- [ ] Provider aliases (`anthropic/google/gemini/deepseek/qwen/glm/zhipu`) resolve.
- [ ] Structured response parsing succeeds for text-only and tool-call responses.

## 7. Observability and Failure Diagnostics

- [ ] Errors contain actionable messages (provider/tool/runtime context).
- [ ] Tool failure guard emits explicit stop reason.
- [ ] Request/session identifiers are available in runtime context where applicable.

## 8. Security and Safety Baseline

- [ ] API keys are not written to logs.
- [ ] Tool policy defaults do not unintentionally widen permissions.
- [ ] Network tool executors have timeout handling.

## 9. Test Coverage Gates

- [ ] Unit tests for provider mapping pass.
- [ ] Existing core tests pass without flaky failures.
- [ ] Manual smoke tests completed for both examples.

## 10. Known Remaining Work (not release blockers unless required by product)

- [ ] Full strict native protocol coverage for all targeted vendors.
- [ ] Structured tool trace persistence inside session history.
- [ ] Expanded automated tests for loop retry/guard branches.
- [ ] Production-grade asynchronous tool execution model.
