# README / 项目说明

## 摘要 / Summary
qt-llmlite 是一个面向 Qt/C++ 的轻量级 LLM 接入库，支持作为 Qt 包复用或源码级集成。

## 项目介绍（基于本次合并更新）
当前版本已从基础聊天库升级为可扩展的会话与工具编排底座，重点能力包括：

- 多客户端工厂与持久化（按 `clientId` 恢复状态）
- 单 client 多历史会话（active + history）
- profile 驱动上下文（系统提示、能力、偏好、风格）
- tools 抽象运行时（registry/selection/adapter/execution/policy/catalog）
- `QtLLMClient` 内建 tool loop 与失败保护
- OpenAI / Anthropic / Google 协议字段映射
- DeepSeek / Qwen / GLM 等别名 provider 接入

## 集成方式
- 作为独立 Qt 包引入
- 作为源码模块直接集成

## 主要文档
- `PROJECT_SPEC.md`
- `ARCHITECTURE.md`
- `RELEASE_READINESS_CHECKLIST.md`
- `AI_RULES.md`
- `CODING_GUIDELINES.md`
- `DECISIONS.md`
