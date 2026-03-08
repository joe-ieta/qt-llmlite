# RELEASE_READINESS_CHECKLIST / 发布前检查清单

## 适用范围

本清单用于当前 qt-llm 基线版本（包含以下能力）：

- 多 Client 工厂与会话持久化
- tools 运行时抽象与内置工具
- QtLLMClient 内建 tool-loop 状态机
- Provider 厂商协议字段映射

## 1. 构建与打包

- [ ] `nmake` 能完整构建库、测试与示例。
- [ ] `simple_chat` 可启动并完成基础非流式请求。
- [ ] `multi_client_chat` 可启动且核心交互稳定。
- [ ] 未引入新的强制外部依赖。

## 2. 基础对话回归

- [ ] 不走 tools 入口时基础链路正常（`ConversationClient -> QtLLMClient`）。
- [ ] 流式 token 顺序正确，`completed` 只触发一次。
- [ ] 错误路径触发 `errorOccurred` 且不会阻塞后续请求。

## 3. 多 Client / 会话持久化

- [ ] 新建 client 生成唯一 `clientId`。
- [ ] 复用已有 `clientId` 可恢复 config/profile/history。
- [ ] 单个 client 同时只有一个 active session。
- [ ] 完成一轮/多轮后可新建 session 形成历史归档。
- [ ] 切换 session 时历史消息显示正确。

## 4. 工具目录与策略持久化

- [ ] 工具目录文件落盘到 `.qtllm/tools/catalog.json`。
- [ ] 内置工具（`current_time`、`current_weather`）启动自动注入。
- [ ] 内置工具为不可删除并强制启用状态。
- [ ] client 工具策略落盘到 `.qtllm/clients/<clientId>/tool_policy.json`。

## 5. 工具运行时行为

- [ ] ToolSelectionLayer 能基于 profile/history 筛选工具。
- [ ] 执行层能执行 allow/deny 及每轮调用上限策略。
- [ ] 默认 dry-run 模式返回结构化失败且不崩溃。
- [ ] 失败保护阈值生效，避免模型死循环调用。
- [ ] loop 上下文正确使用 `clientId + sessionId`。

## 6. Provider 协议映射

- [ ] OpenAI 路径正确构建 `/chat/completions` 并解析 `tool_calls`。
- [ ] Anthropic 映射正确构建 `/messages`、请求头、tool blocks。
- [ ] Google 映射正确构建 `...:generateContent` 与 function-call parts。
- [ ] 别名 provider（`anthropic/google/gemini/deepseek/qwen/glm/zhipu`）可创建。
- [ ] 结构化响应解析在纯文本与含 tool-call 两种场景均可通过。

## 7. 观测与故障定位

- [ ] 错误信息具备可定位上下文（provider/tool/runtime）。
- [ ] tool failure guard 触发时有明确终止原因。
- [ ] runtime context 中可携带 request/session 标识（适用时）。

## 8. 安全与默认策略

- [ ] API Key 不输出到日志。
- [ ] 默认工具策略不会无意放大权限。
- [ ] 网络型工具执行器具备 timeout 处理。

## 9. 测试门禁

- [ ] provider 映射相关单测通过。
- [ ] 核心测试通过且无明显随机失败。
- [ ] 两个示例完成手工 smoke 测试。

## 10. 已知后续工作（按产品要求判断是否阻塞发布）

- [ ] 覆盖更多厂商的严格原生协议适配。
- [ ] 会话内结构化 tool trace 持久化。
- [ ] 扩展 loop 重试/失败保护分支自动化测试。
- [ ] 生产级异步 tool 执行模型增强。
