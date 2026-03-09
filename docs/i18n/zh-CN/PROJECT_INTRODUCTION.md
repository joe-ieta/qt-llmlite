# PROJECT_INTRODUCTION / 项目简介

## 摘要 / Summary
qt-llmlite 是一个面向 Qt/C++ 的轻量级 LLM 接入库。当前版本已覆盖从基础聊天到多客户端会话与 tools 编排的完整基础能力。

## 核心定位
- 作为 Qt 应用的 LLM 基础设施层，而不仅是示例聊天客户端
- 保持 Provider 抽象，支持多模型厂商协议映射
- 为产品化场景提供可扩展的会话与工具运行时能力

## 本次合并后的关键升级
- 多客户端工厂与持久化恢复（`clientId`）
- 单 client 多会话历史管理（`clientId + sessionId`）
- profile 驱动上下文（system prompt / persona / preference / thinking style）
- tools 抽象层与运行时（registry/selection/adapter/execution/policy/catalog）
- `QtLLMClient` 内建 tool loop 状态机与失败保护（防止循环调用）
- 固定内置工具：`current_time`、`current_weather`（不可删除）
- provider 字段映射：OpenAI / Anthropic / Google
- provider 别名接入：DeepSeek / Qwen / GLM 等

## 采用方式
- 作为独立 Qt 包发布与复用
- 作为源码模块集成到现有项目

## 关键价值
- 降低接入门槛
- 降低工程改造成本
- 在不破坏基础聊天路径下扩展工具与策略能力
- 保持 Windows/Linux 一致性
