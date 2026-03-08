#include "toolexecutionlayer.h"

#include "builtinexecutors.h"

#include <QElapsedTimer>

namespace qtllm::tools::runtime {

ToolExecutionLayer::ToolExecutionLayer()
    : m_registry(std::make_shared<ToolExecutorRegistry>())
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
    Q_UNUSED(context)

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

    if (!m_registry) {
        result.errorCode = QStringLiteral("registry_missing");
        result.errorMessage = QStringLiteral("ToolExecutorRegistry is not configured");
        return result;
    }

    const std::shared_ptr<IToolExecutor> executor = m_registry->find(request.toolId);
    if (!executor) {
        result.errorCode = QStringLiteral("executor_missing");
        result.errorMessage = QStringLiteral("No executor registered for tool: ") + request.toolId;
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

} // namespace qtllm::tools::runtime
