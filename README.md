# qt-llmlite (qt-llm)

## 简介 / Introduction

`qt-llmlite` 是一个面向 Qt/C++ 的轻量级 LLM 接入库，当前版本已从“单客户端基础对话”升级为“可生产扩展的多客户端会话与工具编排底座”。

It now provides not only basic chat integration, but also multi-client/session persistence, tool runtime orchestration, and vendor-mapped protocol support.

## 本次版本的主要能力 / What’s New in This Merge

- 多客户端工厂与持久化：按 `clientId` 复用与恢复上下文
- 单 Client 多会话历史：`active session + historical sessions`
- Profile 驱动上下文：系统提示、能力、偏好、思维风格
- Tool 运行时抽象：registry / selection / adapter / execution / policy / catalog
- `QtLLMClient` 内建 tool loop 状态机（含失败保护，防死循环）
- 内置不可删除工具：`current_time`、`current_weather`
- 厂商协议映射：OpenAI / Anthropic / Google + DeepSeek/Qwen/GLM 别名接入
- 新示例：`multi_client_chat`（并发 client + 历史会话 + tools 入口）

## 核心价值 / Core Value

- 对 Qt 应用低侵入接入 LLM
- 从 Demo 到产品化演进路径清晰
- 保持 Provider 抽象边界，便于后续扩展更多模型厂商

## 快速导航 / Quick Navigation

- 项目规格：`PROJECT_SPEC.md`
- 架构说明：`ARCHITECTURE.md`
- 发布前检查：`RELEASE_READINESS_CHECKLIST.md`
- 示例：`src/examples/simple_chat`、`src/examples/multi_client_chat`
- 中文文档：`docs/i18n/zh-CN/`
- English docs: `docs/i18n/en/`

## License

MIT. See `LICENSE`.
