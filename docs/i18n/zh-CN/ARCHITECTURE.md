# ARCHITECTURE / 架构说明

## 摘要 / Summary
qt-llmlite 采用“小型 Qt 库 + 示例应用”的结构，并保持严格依赖方向与全异步请求流程。

## 依赖流向
`Application/UI -> QtLLMClient -> ILLMProvider -> HttpExecutor -> LLM service`

## 架构原则
1. Provider 抽象优先
2. 网络全异步
3. 接口对流式输出友好
4. 模块职责单一

## 模块职责
- `QtLLMClient`：请求编排与对外异步 API
- `ILLMProvider`：Provider 特有请求/解析逻辑
- `HttpExecutor`：底层 HTTP 执行和信号转发
- Stream parser：增量 token 解析
- Example UI：仅作集成示例

## 运行流程
### 非流式
1. UI 发起 prompt
2. Provider 构建请求
3. Executor 发送 HTTP 请求
4. Provider 解析最终响应
5. Client 发出完成信号

### 流式
1. UI 发起 prompt
2. Provider 构建流式请求
3. 网络返回分块
4. 解析器累积分块
5. Provider 解析 token
6. Client 发出 token 与完成信号

## 后续演进
- Provider 工厂
- 对话消息模型
- Embeddings 层
- 配置持久化服务
- Qt Creator 定向集成