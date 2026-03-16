# PROJECT_INTRODUCTION / 项目简介

## 摘要 / Summary
qt-llmlite 是一个面向 Qt/C++ 的轻量级 LLM 接入库。当前版本已从基础聊天扩展为覆盖多客户端会话、完整工具运行时、MCP 集成和统一可观测性的基础能力。

## 核心定位
- 作为 Qt 应用的 LLM 基础设施层，而不仅是示例聊天客户端
- 保持 Provider 抽象，分别支持 OpenAI 与 OpenAI-compatible 路径
- 为产品化场景提供可扩展的会话与工具运行时能力

## 本次合并后的关键升级
- 多客户端工厂与持久化恢复（`clientId`）
- 单 client 多会话历史管理（`clientId + sessionId`）
- profile 驱动上下文（system prompt / persona / preference / thinking style）
- tools 抽象层与运行时（registry/selection/adapter/execution/policy/catalog）
- `QtLLMClient` 内建 tool loop 状态机与失败保护（防止循环调用）
- 面向 OpenAI `Responses API` 的独立 `OpenAIProvider`
- 面向 Ollama 等 `/chat/completions` 风格服务的 `OpenAICompatibleProvider`
- 统一工具注册表，包含内置工具与 MCP 导入工具
- MCP Server 注册、持久化、能力查看、工具同步与执行路由
- 统一日志系统：按 `clientId` 轮换 JSONL 文件并支持 Qt 信号推送
- 示例程序：`simple_chat`、`multi_client_chat`、`mcp_server_manager_demo`

## 采用方式
- 作为独立 Qt 包发布与复用
- 作为源码模块集成到现有项目

## 关键价值
- 降低接入门槛
- 降低工程改造成本
- 在不破坏基础聊天路径下扩展工具与策略能力
- 保持 Windows/Linux 一致性
- 提升 Provider、工具调用和 MCP 链路的调试可观测性
