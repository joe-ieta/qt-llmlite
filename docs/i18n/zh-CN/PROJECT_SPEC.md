# PROJECT_SPEC / 项目规格

## 摘要 / Summary
本文档定义 qt-llm 的当前范围、实现状态、架构边界与后续演进方向。

## 1. 项目概览

- 名称：qt-llmlite（qt-llm）
- 目标：轻量级 Qt/C++ 本地优先 LLM 接入工具库
- 适用：Qt 桌面应用、本地模型服务、OpenAI 兼容及主要厂商远程接口

## 2. 设计目标

1. Qt 原生实现
2. 轻量架构
3. 最小依赖
4. Provider 抽象
5. 本地优先能力
6. 异步网络
7. 流式输出支持
8. 低成本集成
9. 多 Client 会话编排与持久化
10. 具备可生产扩展的 tools/function-calling 能力

## 3. 技术方向

- 语言：C++
- 框架：Qt
- 构建：qmake
- IDE：Qt Creator
- 传输：HTTP
- 基线接口：OpenAI 兼容 `/chat/completions`

约束：

- 核心库保持 Python-free
- 避免引入重型依赖

## 4. Provider 类型

### 本地 Provider

- Ollama
- vLLM
- llama.cpp 兼容服务
- LM Studio 兼容端点

### 远程 Provider

- OpenAI
- Anthropic
- Google
- DeepSeek / Qwen / GLM（通过映射路径接入）

Provider 协议逻辑封装在 `ILLMProvider` 实现中，不进入 UI 层。

## 5. 高层架构

```text
Qt Application/UI
  -> (A) QtLLMClient                    [基础入口 + 可启用内建 tool loop]
  -> (B) ToolEnabledChatEntry           [可选编排入口]
        -> Tool 选择 + 适配 + 策略
        -> QtLLMClient / Provider 抽象
  -> ILLMProvider
  -> HttpExecutor
  -> 本地或远程 LLM 服务
```

支撑模块：

- 配置模型
- 请求/响应类型
- 异步 HTTP 执行
- 流式分片处理
- 错误传播
- Conversation 工厂与持久化
- Tool 注册表/目录/策略/执行/协议路由

## 6. 核心组件

### 6.1 QtLLMClient

- 接收 App 消息
- 持有 active provider 与 executor
- 对外暴露异步 signals/slots API
- 发出 token/completed/error 信号
- 发出结构化响应信号 `responseReceived`
- 在配置 orchestrator + 上下文时执行内建 tool loop（含失败保护）

### 6.2 ConversationClient + ConversationClientFactory

- 管理多个 `clientId`
- 持久化与恢复每个 Client 状态
- 每个 Client 维护：
  - 一个 active session
  - 多个历史 session
- 支持按 `clientId + sessionId` 查看历史
- 发送前向内部 `QtLLMClient` 绑定 tool loop 上下文

### 6.3 ILLMProvider

- Provider 标识
- URL/Headers/Payload 生成
- 最终响应解析
- 流式 token 解析
- 提供结构化 tool-call 支持能力标识

### 6.4 HttpExecutor

- 封装 `QNetworkAccessManager`
- 发出 `dataReceived/requestFinished/errorOccurred`
- 保持 UI 线程友好的异步执行

### 6.5 Tool / Function Calling 抽象层

- `LlmToolRegistry` 维护全局 `llmTools`
- `LlmToolDefinition` 作为内部 canonical schema
- 固定内置工具：`current_time`、`current_weather`
- `ToolSelectionLayer` 每轮选择候选工具
- `ToolCallProtocolRouter` 按模型厂商优先路由
- `ToolExecutionLayer` + executor registry 负责执行抽象
- `ToolCallOrchestrator` 负责工具循环、失败保护与后续提示生成
- `ToolEnabledChatEntry` 作为 App 可选入口与装配层

### 6.6 Tool 持久化与策略

- `ToolCatalogRepository` 持久化工具目录
- `ClientToolPolicyRepository` 持久化每 client 工具策略
- 启动时自动注入并强制保护内置工具（不可删除）

### 6.7 配置模型

- `providerName`, `baseUrl`, `apiKey`, `model`, `modelVendor`, `stream`

## 7. 当前实现状态（截至 2026-03-08）

### 已完成

- qmake 多工程工作区
- `QtLLMClient` 异步编排 + 内建 tool-loop 状态流
- `ILLMProvider` 抽象
- `OpenAICompatibleProvider` 已支持：
  - OpenAI 风格请求/响应/流式解析
  - Anthropic 映射（`/messages`、`tool_use` 块）
  - Google 映射（`generateContent`、`functionCall` parts）
- ProviderFactory 厂商别名入口：OpenAI/Anthropic/Google/DeepSeek/Qwen/GLM 等
- Conversation 工厂与持久化基础：多 Client + 单 active session + 历史 sessions
- tools 运行时栈：
  - registry、built-ins、protocol adapter/router
  - execution layer、orchestrator、catalog/policy repository
- `simple_chat` 继续可用
- `multi_client_chat` 示例已覆盖多 Client + 会话历史 + tools 入口
- 单测已补充 provider 映射与别名创建

### 部分完成 / 待增强

- 更多厂商的严格原生协议覆盖
- 全面稳健的流式分片解析
- 会话内结构化 tool trace 持久化
- tool loop 状态机与持久化链路的更完整自动化测试
- 可复用 Qt 包发布流程

## 8. 当前范围基线

- 核心库：`src/qtllm`
- 多 Client 会话持久化基础
- 厂商映射 provider 路径
- tools 抽象与内置工具能力
- 示例应用：`simple_chat`、`multi_client_chat`
- 中英双语文档

## 9. 路线图（下一阶段）

### Phase 2：协议与运行时增强

- 增加更多厂商协议适配
- 流式解析边界场景加固
- tool-loop 链路下的 timeout/retry/cancel 加固

### Phase 3：会话与追踪增强

- 持久化结构化 tool_call/tool_result 轨迹
- 提升会话回放与排障能力
- 增强工厂/会话/tool-loop 组合测试

### Phase 4：生产化控制

- 工具分类权限与安全策略
- 观测字段与审计追踪
- 模型与工具路由优化

## 10. 约束

- 支持 Windows 与 Linux
- 保持 Qt Creator 友好
- 支持本地/内网模型服务
- 控制编译与依赖开销
- 不假设仅云端部署

## 11. 非目标

- 不实现模型推理本体
- 不嵌入重型服务框架
- 不将 UI 与 Provider 细节耦合

## 12. 对 Coding Agent 的要求

1. 先阅读 `PROJECT_SPEC.md`、`AI_RULES.md`、`ARCHITECTURE.md`、`CODING_GUIDELINES.md`
2. 保持 Provider 抽象边界
3. 保持网络异步
4. 优先增量、可编译变更
5. 避免不必要依赖
6. 保持 Qt Creator 一等支持
7. tools/runtime 变更应保持对基础文本对话路径的兼容
