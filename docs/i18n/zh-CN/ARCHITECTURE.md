# ARCHITECTURE / 架构说明

## 摘要 / Summary

qt-llm 采用“小型 Qt 核心库 + 示例应用”结构。

核心依赖方向：

```text
Application/UI
  -> 编排入口（基础或 tool-enabled）
  -> QtLLMClient
  -> ILLMProvider
  -> HttpExecutor / Qt network layer
  -> 远程或本地 LLM service
```

`QtLLMClient` 仍是基础入口，并在配置 orchestrator/上下文时启用内建 tool-loop 状态处理。

## 架构原则

1. Provider 抽象优先
2. 网络全异步
3. 对流式输出与分片友好
4. 增量演进（能力叠加，不破坏基础路径）
5. 模块职责单一，避免循环依赖

## 目录结构（当前）

```text
src/
  qtllm/
    core/
      qtllmclient.h/.cpp
      llmtypes.h
      llmconfig.h
    chat/
      conversationclient.h/.cpp
      conversationclientfactory.h/.cpp
      conversationsnapshot.h
    profile/
      clientprofile.h
      memorypolicy.h
    storage/
      conversationrepository.h/.cpp
    providers/
      illmprovider.h
      openaicompatibleprovider.h/.cpp
      ollamaprovider.h/.cpp
      vllmprovider.h/.cpp
      providerfactory.h/.cpp
    network/
      httpexecutor.h/.cpp
    streaming/
      streamchunkparser.h/.cpp
    tools/
      llmtooldefinition.h
      llmtoolregistry.h/.cpp
      builtintools.h/.cpp
      llmtooladapter.h/.cpp
      toolselectionlayer.h/.cpp
      toolenabledchatentry.h/.cpp
      protocol/
        itoolcallprotocoladapter.h
        providerprotocoladapters.h/.cpp
        toolcallprotocolrouter.h/.cpp
      runtime/
        toolruntime_types.h
        toolexecutionlayer.h/.cpp
        toolexecutorregistry.h/.cpp
        itoolexecutor.h
        builtinexecutors.h/.cpp
        toolcallorchestrator.h/.cpp
        toolcatalogrepository.h/.cpp
        clienttoolpolicyrepository.h/.cpp
  examples/
    simple_chat/
      main.cpp
      chatwindow.h/.cpp
    multi_client_chat/
      main.cpp
      multiclientwindow.h/.cpp
```

## 运行流程

### A. 基础文本/工具能力流程

1. UI 调用 `QtLLMClient::sendPrompt(...)` 或 `sendRequest(...)`
2. Client 解析 provider 并经 `HttpExecutor` 发起请求
3. Provider 解析最终响应或流式 token
4. 若配置了 orchestrator + 上下文：
   - 先解析 tool calls（结构化优先，文本兜底）
   - 通过 runtime 层执行工具
   - 生成 follow-up 消息继续下一轮
   - 直到完成或触发失败保护
5. Client 发出 `tokenReceived`、`responseReceived`、`completed`、`errorOccurred`

### B. 多 Client / 会话流程

1. UI 通过 `ConversationClientFactory` 按 `clientId` 获取 Client
2. Factory 通过 `ConversationRepository` 读写快照
3. `ConversationClient` 维护 1 个 active session + 多个历史 session
4. UI 按 `sessionId` 新建/切换历史
5. 发送前 `ConversationClient` 向内部 `QtLLMClient` 注入 `clientId + sessionId` tool-loop 上下文

### C. Tool-enabled 入口流程

1. App 可选使用 `ToolEnabledChatEntry`
2. 入口读取 profile/history/tool registry/policy
3. `ToolSelectionLayer` 选择候选工具
4. adapter 把 canonical tools 映射到 provider/model 描述
5. 入口装配 orchestrator/policy 后，通过 `ConversationClient` 发送
6. 实际 loop 在内部 `QtLLMClient` 状态机中执行

## 设计边界

### `QtLLMClient`

负责：

- provider + executor
- 请求生命周期
- stream token 发射
- 可选内建 tool-loop 状态处理

不负责：

- tool catalog/registry 持久化
- UI 渲染
- 业务层产品策略

### `ConversationClient` / `ConversationClientFactory`

负责：

- client 身份生命周期
- 每 client 的 active session 与历史 sessions
- 快照持久化触发
- profile/config 绑定与请求构造

不负责：

- 厂商协议 JSON 细节
- 底层 HTTP 执行

### `ILLMProvider`

负责：

- URL / headers / payload 映射
- 响应解析
- 流式 token 解析
- 厂商协议适配

不负责：

- UI 行为
- 顶层调度或策略编排

### `HttpExecutor`

负责：

- `QNetworkAccessManager`
- HTTP 生命周期
- timeout/retry/cancel

不负责：

- provider/tool 语义解析

### Tools Runtime

负责：

- canonical tool 定义与 registry
- built-in 工具注入与不可删除约束
- 协议适配路由（厂商优先）
- 执行层抽象与策略校验
- loop 编排与失败保护
- tool catalog 与 client policy 持久化

不负责：

- 替换基础聊天 API
- Widget/UI 逻辑

## Canonical Tool 模型

`LlmToolDefinition` 包含：

- `toolId`, `name`, `description`
- `inputSchema`
- `capabilityTags`
- 元数据（`category`、built-in/removable/enabled 标记）

provider adapter 负责 canonical -> vendor descriptor 的映射。

## 持久化模型

### Conversation 快照

按 `clientId` 持久化，包含：

- profile
- provider 配置（`providerName/baseUrl/apiKey/model/modelVendor/stream`）
- active session id
- session 列表与消息历史

### Tools 数据

- `ToolCatalogRepository` 持久化工具目录
- `ClientToolPolicyRepository` 持久化 client 工具策略
- 启动时强制合并内置工具

## 当前缺口与增强方向

- 更多厂商的严格原生协议覆盖
- 会话层结构化 tool trace 持久化
- tool-loop 分支（失败保护/重试/异常）测试覆盖增强
- 生产级观测与审计字段进一步标准化
