# PROJECT_SPEC / 项目规格

## 摘要 / Summary
本文档定义 qt-llmlite 的范围、目标、架构方向与阶段交付计划。

## 1. 项目概览
- 名称：qt-llmlite（qt-llm）
- 目标：提供轻量级 Qt/C++ LLM 接入能力
- 范围：本地优先，同时支持 OpenAI 兼容远程接口

## 2. 设计目标
1. Qt 原生实现
2. 轻量架构
3. 最小依赖
4. Provider 抽象解耦
5. 异步网络
6. 流式输出支持
7. 低成本集成

## 3. 技术方向
- 语言：C++
- 框架：Qt
- 构建系统：qmake（当前基线）
- IDE：Qt Creator
- 传输：HTTP API

## 4. Provider 类型
- 本地：Ollama、vLLM、llama.cpp 兼容、LM Studio 兼容
- 远程：OpenAI 与其他 OpenAI 兼容服务

## 5. 高层架构
`Qt Application -> QtLLMClient -> ILLMProvider -> HttpExecutor -> LLM service`

## 6. 核心组件
- `QtLLMClient`：请求编排与异步信号 API
- `ILLMProvider`：URL/头/负载/解析的 Provider 逻辑
- `HttpExecutor`：基于 `QNetworkAccessManager` 的异步执行
- Streaming parser：分块与增量 token 解析
- Config model：基础配置模型

## 7. 编码原则
- 接口小而清晰
- Provider 逻辑保持解耦
- 禁止阻塞 UI 线程
- 优先可编译的增量变更

## 8. 初始范围（Phase 1）
- `QtLLMClient`
- `ILLMProvider`
- OpenAI 兼容 Provider
- 基础 HTTP 执行器
- 示例应用

## 9. 路线图
- Phase 1：基础能力
- Phase 2：Provider 扩展与配置持久化
- Phase 3：抽象增强与可用性提升
- Phase 4：高级集成与 Qt Creator 方向

## 10. 约束
- 必须支持 Windows 与 Linux
- 控制编译和依赖开销
- 支持内网/本地模型服务

## 11. 非目标
- 不实现模型推理本身
- 核心 C++ 库不依赖 Python 运行时
- 不嵌入重型服务框架

## 12. 对编码代理的要求
- 先阅读核心文档
- 保持抽象边界
- 保持网络异步
- 变更小而聚焦