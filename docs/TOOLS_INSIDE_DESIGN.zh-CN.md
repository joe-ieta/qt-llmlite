# TOOLS_INSIDE_DESIGN.zh-CN.md

## 1. 背景

当前 `QtLLMClient` 已具备内部 tool loop 能力，主链路为：

- `ToolEnabledChatEntry` 选择工具并构造 provider-facing tools schema
- `ConversationClient` 组织会话上下文并发起请求
- `QtLLMClient` 调度 provider / HTTP executor，并在响应完成后触发内部 tool loop
- `ToolCallOrchestrator` 解析模型返回的 tool calls，执行失败保护与 follow-up prompt 生成
- `ToolExecutionLayer` 执行内建工具与 MCP 工具

现有问题不是“没有日志”，而是：

- 模型推理、多轮工具调用、follow-up prompt、失败保护之间缺少统一的可追踪事件模型
- 关键步骤、工具执行、失败原因、重试与时序分散在普通日志里，不易重建完整链路
- 当前持久化重点是会话历史与通用日志，不足以支撑“工具结果如何支撑模型输出”的分析
- 现有日志更适合排障，不适合做跨 client / session / trace 的聚合、筛选、归档和可视化

## 2. 核心目标

`tools_inside` 的核心目标不是简单查看调用时序，而是分析：

- 一次用户问题触发了哪些工具调用
- 工具调用如何按时间顺序参与推理过程
- 哪些工具结果被注入回模型
- 哪些工具结果对最终模型输出形成了直接或间接支撑
- 哪些工具调用是无效、失败、冗余或被后续推理忽略的

因此，`tools_inside` 需要同时具备：

- 结构化追踪
- 本地完整持久化
- 分层查询
- 时间轴可视化
- 支撑关系分析
- 归档与清理能力

## 3. 本次达成的共识

### 3.1 总体原则

- 新增独立的 trace / event 机制，不在普通日志系统上直接堆功能
- 现有日志系统继续保留，作为人类排障入口
- `tools_inside` 作为独立观测与分析体系，优先通过旁路埋点接入，不重写现有 tool / model 主流程
- 首版优先打通“采集 -> 存储 -> 查询 -> 可视化”的主链路

### 3.2 数据保留原则

- prompt、request payload、provider payload、tool arguments、tool output、raw response 要完整持久化到本地
- 脱敏不作为首版重点，但必须预留统一策略接口和钩子
- 历史归档与清理必须纳入设计，从一开始就通过 API 层封装

### 3.3 可视化原则

- 按 `client -> session -> trace/tool call` 三层组织
- 必须清晰表达工具调用时序
- 必须在时间轴上展现 request、tool batch、tool call、follow-up prompt、final answer 的先后关系
- 可视化工具独立存在，首版位置为 `src/examples/tools_inside`
- 技术栈优先使用 `QML / Qt Quick`

### 3.4 分析原则

- “分析各工具调用结果对模型输出的支撑”是首要目标
- 不能只记录调用是否发生，还必须记录：
  - 模型请求中携带了哪些工具
  - 模型返回了哪些 tool calls
  - 工具执行返回了哪些结构化结果
  - 哪些结果被注入 follow-up prompt
  - 最终回答与哪些工具结果存在支撑关系

## 4. 不采用的方案

以下方案本期不采用：

- 仅依赖现有 `LogEvent` JSONL 日志做离线解析
- 只做一个实时日志查看器而不做本地持久化模型
- 将所有大对象直接塞入单一数据库表
- 直接修改 provider / executor 主体行为，把观测逻辑耦合进业务分支
- 首版投入大量精力做复杂脱敏或云端分析

## 5. 架构决策

### 5.1 决策一：观测体系独立于日志体系

保留现有 `QtLlmLogger`、`FileLogSink`、`SignalLogSink` 作为通用日志能力。

新增独立的 `tools_inside` 追踪体系，用于：

- 结构化事件采集
- 本地持久化
- 查询与归档
- 可视化分析

日志仍可引用相同上下文：

- `clientId`
- `sessionId`
- `requestId`
- `traceId`

但日志不是分析主存储。

### 5.2 决策二：采用 SQLite + artifact file 双层存储

`tools_inside` 存储分为两部分：

- 索引、关系、时间轴、筛选查询使用 SQLite
- 大对象内容使用 artifact 文件存储

原因：

- SQLite 适合多维查询、时间排序、聚合和归档状态管理
- prompt、payload、raw response、tool output 可能较大，不适合全部内联到同一张表
- 双层结构便于后续归档、清理和多端 API 复用

### 5.3 决策三：通过 API 层统一封装访问

不允许 UI 直接访问数据库和 artifact 文件。

必须提供一层 API 层，负责：

- 写入
- 查询
- 归档
- 清理
- 后续脱敏策略接入

这样后续可以复用到：

- QML 本地桌面工具
- Qt Widget 辅助面板
- 潜在的远程服务或多端展示

### 5.4 决策四：首版优先恢复“工具结果真正回注模型”的链路语义

当前 follow-up prompt 构建中，工具执行结果没有以完整语义回注模型。

首版实现 `tools_inside` 时，必须同步修正：

- follow-up prompt 对 `ToolExecutionResult.output` 的真实注入
- provider payload 观测链路补全

否则“工具结果支撑模型输出”的分析无法成立。

## 6. 分层设计

建议新增模块：`src/qtllm/toolsinside/`

模块划分如下。

### 6.1 Model 层

负责定义稳定的数据模型：

- `ToolsInsideTraceRecord`
- `ToolsInsideSpanRecord`
- `ToolsInsideEventRecord`
- `ToolsInsideToolCallRecord`
- `ToolsInsideArtifactRef`
- `ToolsInsideSupportLink`
- `ToolsInsideArchivePolicy`
- `ToolsInsideStoragePolicy`

### 6.2 Recorder 层

负责从现有运行时关键节点采集结构化事件：

- `ToolsInsideTraceRecorder`
- `ToolsInsideSpanScope`
- `ToolsInsideArtifactWriter`

特点：

- 旁路接入
- 尽量不改变原执行语义
- 与通用 logger 解耦

### 6.3 Storage 层

负责数据库与 artifact 管理：

- `ToolsInsideDatabase`
- `ToolsInsideRepository`
- `ToolsInsideArtifactStore`

### 6.4 Query API 层

负责统一查询和管理操作：

- `ToolsInsideQueryService`
- `ToolsInsideRetentionService`
- `ToolsInsideAdminService`

### 6.5 UI 层

独立工具位置：

- `src/examples/tools_inside`

首版采用：

- `QML / Qt Quick`

## 7. 关键标识与层级模型

`tools_inside` 使用以下层级组织：

- Client 层：`clientId`
- Session 层：`sessionId`
- Trace 层：`traceId`
- Request 层：`requestId`
- Span/Event 层：`spanId` / `eventId`
- Tool Call 层：`toolCallId`

建议语义：

- 一次用户输入触发一次 `trace`
- 一个 `trace` 内可能包含多次 `request`
- 每次 `request` 可能产生一个或多个 `tool batch`
- 每个 `tool batch` 内包含多个 `tool call`
- 一个 `trace` 最终结束于：
  - 成功完成
  - provider/network error
  - tool failure guard
  - 用户取消

## 8. 数据模型设计

### 8.1 Trace

表示一次完整用户问题处理过程。

核心字段建议：

- `traceId`
- `clientId`
- `sessionId`
- `rootRequestId`
- `turnInputArtifactId`
- `startedAtUtc`
- `endedAtUtc`
- `status`
- `model`
- `provider`
- `vendor`
- `toolSelectionSummary`

### 8.2 Span

表示持续时间节点。

建议 span 类型：

- `user_turn`
- `tool_selection`
- `llm_request`
- `response_parse`
- `tool_loop_round`
- `tool_batch`
- `tool_call`
- `followup_prompt_build`
- `final_answer_build`

核心字段建议：

- `spanId`
- `traceId`
- `parentSpanId`
- `requestId`
- `toolCallId`
- `kind`
- `name`
- `startedAtUtc`
- `endedAtUtc`
- `status`
- `errorCode`
- `summaryJson`

### 8.3 Event

表示瞬时节点。

建议 event 类型：

- `turn_started`
- `tool_selection_completed`
- `request_prepared`
- `provider_payload_built`
- `request_dispatched`
- `stream_first_content_token`
- `stream_first_reasoning_token`
- `response_received`
- `tool_calls_detected`
- `tool_execution_started`
- `tool_execution_finished`
- `followup_prompt_built`
- `failure_guard_triggered`
- `trace_completed`
- `trace_failed`

### 8.4 Tool Call Record

面向后续分析的投影结构，专门存工具调用。

核心字段建议：

- `toolCallId`
- `traceId`
- `requestId`
- `roundIndex`
- `toolId`
- `toolName`
- `toolCategory`
- `serverId`
- `invocationName`
- `argumentsArtifactId`
- `outputArtifactId`
- `followupArtifactId`
- `status`
- `errorCode`
- `errorMessage`
- `retryable`
- `durationMs`

### 8.5 Artifact

表示大对象内容。

artifact 类型建议：

- `turn_input`
- `request_payload`
- `provider_payload`
- `raw_response`
- `assistant_response`
- `tool_arguments`
- `tool_output`
- `followup_prompt`
- `final_answer`
- `support_evidence`

### 8.6 Support Link

表示工具结果对模型输出的支撑关系。

字段建议：

- `supportLinkId`
- `traceId`
- `toolCallId`
- `targetKind`
- `targetId`
- `supportType`
- `confidence`
- `source`
- `evidenceArtifactId`
- `note`

`supportType` 建议枚举：

- `consumed_by_followup`
- `referenced_in_answer`
- `fact_source`
- `decision_basis`
- `unused`
- `contradicted`
- `manual_annotation`

## 9. 存储架构

### 9.1 目录布局

建议运行时目录：

```text
.qtllm/
  tools_inside/
    index.db
    artifacts/
      <clientId>/
        <sessionId>/
          <traceId>/
            <artifact files>
    archive/
      <archive sets>
```

### 9.2 数据库表建议

建议表：

- `ti_clients`
- `ti_sessions`
- `ti_traces`
- `ti_spans`
- `ti_events`
- `ti_tool_calls`
- `ti_artifacts`
- `ti_support_links`
- `ti_archive_jobs`

### 9.3 持久化策略

首版策略：

- 所有 trace 元数据写入 SQLite
- 所有原始大对象写入 artifact 文件
- 数据库只保存 artifact 元信息与路径

优点：

- 查询高效
- 可独立归档 artifact
- 可保留数据库级历史索引

## 10. API 层设计

`tools_inside` 需要一层稳定 API，供后续 UI 和其他端使用。

建议最小接口：

### 10.1 Query API

- `listClients()`
- `listSessions(clientId)`
- `listTraces(clientId, sessionId, filter)`
- `getTraceSummary(traceId)`
- `getTraceTimeline(traceId)`
- `getTraceArtifacts(traceId)`
- `listToolCalls(filter)`
- `getToolCallDetail(toolCallId)`
- `listFailures(filter)`
- `listSupportLinks(traceId)`

### 10.2 Admin API

- `archiveTrace(traceId, options)`
- `archiveSession(clientId, sessionId, options)`
- `purgeTrace(traceId)`
- `purgeSession(clientId, sessionId)`
- `getStorageStats(scope)`

### 10.3 Annotation API

- `saveSupportAnnotation(traceId, toolCallId, annotation)`
- `updateSupportAnnotation(linkId, annotation)`
- `removeSupportAnnotation(linkId)`

## 11. 归档与清理设计

### 11.1 目标

本地完整持久化会带来长期存储膨胀，因此必须具备：

- trace 级归档
- session 级归档
- 清理未使用 artifact
- 按时间范围清理

### 11.2 设计原则

- 归档和清理都通过 API 层执行
- UI 不能直接删除数据库或文件
- 默认先做手动触发
- 自动归档策略可以后续扩展

### 11.3 状态建议

trace / artifact 状态建议包括：

- `active`
- `archived`
- `purged`
- `orphaned`

## 12. 脱敏与存储策略钩子

本期不把精力放在复杂脱敏能力上，但必须预留扩展点。

建议接口：

- `IToolsInsideRedactionPolicy`
- `IToolsInsideStoragePolicy`

默认首版行为：

- 本地完整存储
- 不脱敏
- 所有写入仍然统一走策略接口

后续可扩展：

- 对 prompt 做规则脱敏
- 对 payload 做字段裁剪
- 对 tool output 做按工具分类处理
- 对导出内容做单独脱敏

## 13. 运行时埋点计划

### 13.1 `ToolEnabledChatEntry`

记录：

- turn started
- tool selection start / end
- selected tools
- schema prepared

### 13.2 `ConversationClient`

记录：

- request prepared
- 当前 session 关联
- 用户 turn 输入 artifact

### 13.3 `QtLLMClient`

记录：

- request dispatched
- network target
- provider payload artifact
- stream first token
- response parsed
- follow-up request dispatched
- completed / error

### 13.4 `ToolCallOrchestrator`

记录：

- protocol adapter resolved
- tool calls parsed
- round index
- failure count
- failure guard triggered
- follow-up prompt built

### 13.5 `ToolExecutionLayer`

记录：

- tool batch started / finished
- tool call started / finished
- built-in / MCP 分支信息
- duration
- success / failure reason

## 14. 对“支撑分析”的首版定义

首版不追求复杂语义理解，先建立可证明的依赖链。

首版必须可靠回答的问题：

- 最终答案之前，模型经历了几轮 request
- 每轮 request 中请求了哪些工具
- 每个工具返回了什么结果
- 哪些结果被注入到了 follow-up prompt
- 最终答案属于哪条 trace
- 哪些工具结果在该 trace 中被模型实际消费过

首版可以提供的支撑分类：

- 工具结果已执行，但未回注模型
- 工具结果已回注模型，但后续答案未体现
- 工具结果已回注模型，且后续答案存在明显引用
- 工具结果失败，导致后续推理降级或终止

更复杂的“事实支撑评分”与“自动质量评估”放到后续迭代。

## 15. 独立可视化工具设计

### 15.1 工具位置

- `src/examples/tools_inside`

### 15.2 技术选型

- 首版：`QML / Qt Quick`

原因：

- 时间轴、泳道、层级树、详情面板更适合 QML
- 保持 Qt 原生轻量，不依赖 Web runtime
- 后续如需 WebView 或多端，可以复用 API 层

### 15.3 UI 结构

左侧：

- client 列表
- session 列表
- trace 列表

中间：

- timeline / swimlane

右侧：

- trace summary
- request / payload
- tool arguments / output
- support links
- final answer
- raw artifact viewer
- archive / purge actions

### 15.4 时间轴要求

时间轴必须至少显示：

- 用户输入起点
- 每次 LLM request 的开始与结束
- 每轮 tool loop round
- 每个 tool batch
- 每个 tool call 的开始、结束、成功失败
- follow-up prompt 构造节点
- final answer 输出节点
- failure guard / error 节点

## 16. 实施顺序

### Phase A: 文档与骨架

- 固化本设计文档
- 建立 `tools_inside` 模块目录骨架
- 约定命名、目录与 API 边界

### Phase B: 采集与持久化主链路

- 建立 trace model
- 建立 SQLite + artifact store
- 建立 recorder
- 在关键节点打点
- 打通 traceId 贯通

### Phase C: 语义补强

- 修正 follow-up prompt 注入逻辑
- 补全 provider payload 观测
- 将 tool output 与 follow-up prompt 建立可追踪映射

### Phase D: 查询 API

- 实现 query / admin / retention 基础接口
- 实现 trace timeline 查询
- 实现 tool call detail 查询

### Phase E: 可视化首版

- 建立 `src/examples/tools_inside`
- 列表视图
- 时间轴视图
- 详情面板
- 手动归档 / 清理入口

## 17. 首版范围控制

为避免失控，首版明确不做：

- 复杂自动脱敏
- 云端同步
- 跨机器共享索引
- 自动 LLM 评估
- 高级统计报表
- 复杂权限系统

首版要做好的只有四件事：

- 采集完整
- 持久化完整
- 查询清晰
- 时间轴与支撑链可看懂

## 18. 需要同步修正的现有链路问题

本设计落地时，应同步修正以下问题：

- follow-up prompt 应回注真实工具结果，而不是空内容
- `providerPayloadPrepared` 观测链路需要真正接通
- `traceId` 需要从一次用户 turn 开始稳定生成并贯穿整条链路
- 会话历史与 trace 记录应解耦，避免把内部追踪强行塞回普通会话消息

## 19. 结论

`tools_inside` 不是“更详细的日志查看器”，而是一套独立的、产品化的本地工具调用观测与分析能力。

其设计核心是：

- 使用独立 trace 机制，而不是直接堆日志
- 使用 SQLite + artifact file 双层存储
- 使用 API 层隔离存储与可视化
- 使用 `client / session / trace / tool call` 四级结构重建时间轴
- 围绕“工具结果如何支撑模型输出”建立长期可扩展的数据基础

该文档作为 `tools-logical` 分支后续实现的共识基线。
