# PROJECT_SPEC / 项目规格

## 摘要
本文档定义 qt-llm 当前的范围、架构方向与实现状态。

## 1. 项目概览
- 名称：qt-llmlite（qt-llm）
- 目标：轻量级 Qt/C++ 本地优先与远程 LLM 集成工具包
- 适用：Qt 桌面应用、本地模型服务、厂商 API、OpenAI 兼容服务

## 2. 设计目标
1. Qt 原生实现
2. 轻量架构
3. 最小依赖
4. Provider 抽象
5. 异步网络
6. 流式输出支持
7. 易于集成
8. 多 Client 持久化
9. tools/function-calling 支持
10. MCP 集成
11. 结构化可观测性

## 3. 技术方向
- 语言：C++17
- 框架：Qt
- 构建：qmake
- IDE：Qt Creator
- 传输：HTTP
- OpenAI 路径：`/responses`
- OpenAI 兼容路径：`/chat/completions`

## 4. 核心架构
`Qt Application -> ToolEnabledChatEntry 或 ConversationClient -> QtLLMClient -> ILLMProvider -> HttpExecutor -> LLM 服务`

## 5. 当前状态（2026-03-15）
### 已完成
- 独立 `OpenAIProvider`
- `OpenAICompatibleProvider`
- `OllamaProvider` / `VllmProvider`
- 多 Client / Session 持久化
- 内部 tool loop
- 含内置工具的统一工具注册表
- MCP Server 注册、持久化、同步与执行路由
- 文件日志 + UI 信号日志
- 示例应用：`simple_chat`、`multi_client_chat`、`mcp_server_manager_demo`

### 待加固
- MCP 与 OpenAI Responses API 边界场景的自动化测试
- 会话内结构化 tool trace 持久化
- 更完善的外部工具异步执行模型

## 6. Canonical Tool 模型
- `toolId`：内部唯一标识
- `invocationName`：发给模型的函数名
- `name`：UI 展示名

## 7. 约束
- 支持 Windows 与 Linux
- 保持 Qt Creator 友好
- 保持低依赖开销
- 适合本地与内网部署

## 8. 非目标
- 不实现模型推理本体
- 不嵌入重量级 Serving 框架
- 不将 UI 与 Provider 紧耦合
