#include "toolexecutionlayer.h"

#include "builtinexecutors.h"
#include "../builtintools.h"
#include "../mcp/defaultmcpclient.h"
#include "../mcp/mcpserverregistry.h"

#include <QElapsedTimer>

namespace qtllm::tools::runtime {

ToolExecutionLayer::ToolExecutionLayer()
    : m_registry(std::make_shared<ToolExecutorRegistry>())
    , m_mcpClient(std::make_shared<mcp::DefaultMcpClient>())
{
    m_registry->registerExecutor(std::make_shared<CurrentTimeToolExecutor>());
    m_registry->registerExecutor(std::make_shared<CurrentWeatherToolExecutor>());
}

void ToolExecutionLayer::setRegistry(const std::shared_ptr<ToolExecutorRegistry> &registry)
{
    if (registry) {
        m_registry = registry;
    }
}

void ToolExecutionLayer::setPolicy(const ToolExecutionPolicy &policy)
{
    m_policy = policy;
}

void ToolExecutionLayer::setHooks(const std::shared_ptr<ToolRuntimeHooks> &hooks)
{
    m_hooks = hooks;
}

void ToolExecutionLayer::setDryRunFailureMode(bool enabled)
{
    m_dryRunFailureMode = enabled;
}

void ToolExecutionLayer::setMcpClient(const std::shared_ptr<mcp::IMcpClient> &mcpClient)
{
    m_mcpClient = mcpClient;
}

void ToolExecutionLayer::setMcpServerRegistry(const std::shared_ptr<mcp::McpServerRegistry> &serverRegistry)
{
    m_mcpServerRegistry = serverRegistry;
}

QList<ToolExecutionResult> ToolExecutionLayer::executeBatch(const QList<ToolCallRequest> &requests,
                                                            const ToolExecutionContext &context,
                                                            const ClientToolPolicy &clientPolicy) const
{
    QList<ToolExecutionResult> results;
    if (requests.isEmpty()) {
        return results;
    }

    const int limit = qMax(1, clientPolicy.maxToolsPerTurn > 0 ? clientPolicy.maxToolsPerTurn : m_policy.maxParallelCalls);
    const int count = qMin(limit, requests.size());

    for (int i = 0; i < count; ++i) {
        ToolExecutionResult result = executeSingle(requests.at(i), context, clientPolicy);
        results.append(result);

        if (m_policy.failFast && !result.success) {
            break;
        }
    }

    return results;
}

bool ToolExecutionLayer::cancelBySession(const QString &clientId, const QString &sessionId) const
{
    Q_UNUSED(clientId)
    Q_UNUSED(sessionId)
    // TODO: keep running-call index when asynchronous execution is introduced.
    return false;
}

ToolExecutionResult ToolExecutionLayer::executeSingle(const ToolCallRequest &request,
                                                      const ToolExecutionContext &context,
                                                      const ClientToolPolicy &clientPolicy) const
{
    ToolExecutionResult result;
    result.callId = request.callId;
    result.toolId = request.toolId;

    if (!m_policy.isToolAllowed(request.toolId, clientPolicy)) {
        result.errorCode = QStringLiteral("tool_denied");
        result.errorMessage = QStringLiteral("Tool is not allowed by client policy");
        return result;
    }

    if (m_dryRunFailureMode) {
        result.errorCode = QStringLiteral("tool_unavailable");
        result.errorMessage = QStringLiteral("Tool framework is active but real execution is not enabled");
        result.retryable = false;
        return result;
    }

    if (request.toolId.startsWith(QStringLiteral("mcp::"))) {
        return executeMcpTool(request, context);
    }

    if (!m_registry) {
        result.errorCode = QStringLiteral("registry_missing");
        result.errorMessage = QStringLiteral("ToolExecutorRegistry is not configured");
        return result;
    }

    const std::shared_ptr<IToolExecutor> executor = m_registry->find(request.toolId);
    if (!executor) {
        if (!tools::isBuiltInToolId(request.toolId)) {
            // Pseudo flow for non-built-in, non-MCP tools:
            // 1) Resolve external tool metadata by toolId from runtime catalog/registry.
            // 2) Route call to corresponding external adapter.
            // 3) Normalize adapter output into ToolExecutionResult.
            result.errorCode = QStringLiteral("external_executor_not_implemented");
            result.errorMessage = QStringLiteral("External tool execution is not implemented for tool: ") + request.toolId;
            result.retryable = false;
            return result;
        }

        result.errorCode = QStringLiteral("executor_missing");
        result.errorMessage = QStringLiteral("No executor registered for built-in tool: ") + request.toolId;
        return result;
    }

    if (m_hooks) {
        m_hooks->beforeExecute(request, context);
    }

    QElapsedTimer timer;
    timer.start();

    ToolExecutionResult executed = executor->execute(request, context);
    if (executed.callId.isEmpty()) {
        executed.callId = request.callId;
    }
    if (executed.toolId.isEmpty()) {
        executed.toolId = request.toolId;
    }
    executed.durationMs = timer.elapsed();

    if (m_hooks) {
        m_hooks->afterExecute(executed, context);
    }

    return executed;
}

ToolExecutionResult ToolExecutionLayer::executeMcpTool(const ToolCallRequest &request,
                                                       const ToolExecutionContext &context) const
{
    ToolExecutionResult result;
    result.callId = request.callId;
    result.toolId = request.toolId;

    const QStringList parts = request.toolId.split(QStringLiteral("::"));
    if (parts.size() < 3) {
        result.errorCode = QStringLiteral("mcp_tool_id_invalid");
        result.errorMessage = QStringLiteral("Invalid MCP tool id, expected mcp::<serverId>::<toolName>");
        result.retryable = false;
        return result;
    }

    const QString serverId = parts.at(1).trimmed().toLower();
    const QString toolName = parts.mid(2).join(QStringLiteral("::"));

    if (!m_mcpServerRegistry) {
        result.errorCode = QStringLiteral("mcp_server_registry_missing");
        result.errorMessage = QStringLiteral("MCP server registry is not configured");
        result.retryable = false;
        return result;
    }

    const std::optional<mcp::McpServerDefinition> serverOpt = m_mcpServerRegistry->find(serverId);
    if (!serverOpt.has_value()) {
        result.errorCode = QStringLiteral("mcp_server_not_found");
        result.errorMessage = QStringLiteral("MCP server not found: ") + serverId;
        result.retryable = false;
        return result;
    }

    if (!serverOpt->enabled) {
        result.errorCode = QStringLiteral("mcp_server_disabled");
        result.errorMessage = QStringLiteral("MCP server is disabled: ") + serverId;
        result.retryable = false;
        return result;
    }

    if (!m_mcpClient) {
        result.errorCode = QStringLiteral("mcp_client_missing");
        result.errorMessage = QStringLiteral("MCP client is not configured");
        result.retryable = false;
        return result;
    }

    if (m_hooks) {
        m_hooks->beforeExecute(request, context);
    }

    QElapsedTimer timer;
    timer.start();

    QString errorMessage;
    const mcp::McpToolCallResult call = m_mcpClient->callTool(*serverOpt,
                                                               toolName,
                                                               request.arguments,
                                                               context,
                                                               &errorMessage);

    result.durationMs = timer.elapsed();
    result.success = call.success;
    result.output = call.output;
    result.errorCode = call.errorCode;
    result.errorMessage = call.errorMessage.isEmpty() ? errorMessage : call.errorMessage;
    result.retryable = call.retryable;

    if (m_hooks) {
        m_hooks->afterExecute(result, context);
    }

    return result;
}

} // namespace qtllm::tools::runtime
