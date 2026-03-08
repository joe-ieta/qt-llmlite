#include "toolcallorchestrator.h"

#include <optional>

namespace qtllm::tools::runtime {

ToolCallOrchestrator::ToolCallOrchestrator()
    : m_executionLayer(std::make_shared<ToolExecutionLayer>())
    , m_protocolRouter(std::make_shared<protocol::ToolCallProtocolRouter>())
{
}

void ToolCallOrchestrator::setExecutionLayer(const std::shared_ptr<ToolExecutionLayer> &executionLayer)
{
    if (executionLayer) {
        m_executionLayer = executionLayer;
    }
}

void ToolCallOrchestrator::setPolicyRepository(const std::shared_ptr<ClientToolPolicyRepository> &policyRepository)
{
    m_policyRepository = policyRepository;
}

void ToolCallOrchestrator::setMaxConsecutiveFailures(int maxConsecutiveFailures)
{
    m_maxConsecutiveFailures = qMax(1, maxConsecutiveFailures);
}

void ToolCallOrchestrator::setMaxRounds(int maxRounds)
{
    m_maxRounds = qMax(1, maxRounds);
}

void ToolCallOrchestrator::resetSession(const QString &clientId, const QString &sessionId)
{
    const QString key = clientId.trimmed() + QStringLiteral("::") + sessionId.trimmed();
    m_stateBySession.remove(key);
}

ToolLoopOutcome ToolCallOrchestrator::processAssistantMessage(const QString &modelName,
                                                              const QString &modelVendor,
                                                              const QString &providerName,
                                                              const QString &assistantText,
                                                              const ToolExecutionContext &context) const
{
    ToolLoopOutcome outcome;
    if (!m_protocolRouter || !m_executionLayer) {
        return outcome;
    }

    const std::shared_ptr<protocol::IToolCallProtocolAdapter> adapter =
        m_protocolRouter->route(modelName, modelVendor, providerName);
    if (!adapter) {
        return outcome;
    }

    const QList<ToolCallRequest> requests = adapter->parseToolCalls(assistantText);
    return processToolCalls(adapter, requests, assistantText, context);
}

ToolLoopOutcome ToolCallOrchestrator::processAssistantResponse(const QString &modelName,
                                                               const QString &modelVendor,
                                                               const QString &providerName,
                                                               const LlmResponse &response,
                                                               const ToolExecutionContext &context) const
{
    ToolLoopOutcome outcome;
    if (!m_protocolRouter || !m_executionLayer) {
        return outcome;
    }

    const std::shared_ptr<protocol::IToolCallProtocolAdapter> adapter =
        m_protocolRouter->route(modelName, modelVendor, providerName);
    if (!adapter) {
        return outcome;
    }

    QList<ToolCallRequest> requests;
    const QVector<LlmToolCall> toolCalls = response.assistantMessage.toolCalls;
    for (const LlmToolCall &call : toolCalls) {
        ToolCallRequest request;
        request.callId = call.id;
        request.toolId = call.name;
        request.arguments = call.arguments;
        if (!request.toolId.trimmed().isEmpty()) {
            requests.append(request);
        }
    }

    if (requests.isEmpty()) {
        requests = adapter->parseToolCalls(response.text);
    }

    return processToolCalls(adapter, requests, response.text, context);
}

ToolLoopOutcome ToolCallOrchestrator::processToolCalls(const std::shared_ptr<protocol::IToolCallProtocolAdapter> &adapter,
                                                       const QList<ToolCallRequest> &requests,
                                                       const QString &assistantText,
                                                       const ToolExecutionContext &context) const
{
    ToolLoopOutcome outcome;

    const QString key = stateKey(context);
    if (key.isEmpty()) {
        return outcome;
    }

    ToolLoopState state = m_stateBySession.value(key);
    if (state.rounds >= m_maxRounds) {
        outcome.terminatedByFailureGuard = true;
        outcome.consecutiveFailures = state.consecutiveFailures;
        return outcome;
    }

    if (requests.isEmpty()) {
        state.rounds = 0;
        state.consecutiveFailures = 0;
        m_stateBySession.insert(key, state);
        return outcome;
    }

    ClientToolPolicy policy;
    policy.clientId = context.clientId;
    if (m_policyRepository) {
        const std::optional<ClientToolPolicy> loaded = m_policyRepository->loadPolicy(context.clientId, nullptr);
        if (loaded.has_value()) {
            policy = *loaded;
        }
    }

    const QList<ToolExecutionResult> results = m_executionLayer->executeBatch(requests, context, policy);
    int failureCount = 0;
    for (const ToolExecutionResult &result : results) {
        if (!result.success) {
            ++failureCount;
        }
    }

    state.rounds += 1;
    if (!results.isEmpty() && failureCount == results.size()) {
        state.consecutiveFailures += 1;
    } else {
        state.consecutiveFailures = 0;
    }

    m_stateBySession.insert(key, state);

    outcome.consecutiveFailures = state.consecutiveFailures;
    if (state.consecutiveFailures >= m_maxConsecutiveFailures || state.rounds >= m_maxRounds) {
        outcome.terminatedByFailureGuard = true;
        return outcome;
    }

    outcome.hasFollowUpPrompt = true;
    outcome.followUpPrompt = adapter->buildFollowUpPrompt(assistantText, results);
    return outcome;
}

QString ToolCallOrchestrator::stateKey(const ToolExecutionContext &context) const
{
    const QString clientId = context.clientId.trimmed();
    const QString sessionId = context.sessionId.trimmed();
    if (clientId.isEmpty() || sessionId.isEmpty()) {
        return QString();
    }
    return clientId + QStringLiteral("::") + sessionId;
}

} // namespace qtllm::tools::runtime
