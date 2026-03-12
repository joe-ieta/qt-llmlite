#include "toolenabledchatentry.h"

#include <QDateTime>
#include <optional>

namespace qtllm::tools {

ToolEnabledChatEntry::ToolEnabledChatEntry(const QSharedPointer<chat::ConversationClient> &client,
                                           std::shared_ptr<LlmToolRegistry> toolRegistry,
                                           QObject *parent)
    : QObject(parent)
    , m_client(client)
    , m_toolRegistry(std::move(toolRegistry))
    , m_toolAdapter(std::make_unique<DefaultLlmToolAdapter>())
    , m_executionLayer(std::make_shared<runtime::ToolExecutionLayer>())
    , m_orchestrator(std::make_shared<runtime::ToolCallOrchestrator>())
{
    if (m_orchestrator) {
        m_orchestrator->setExecutionLayer(m_executionLayer);
    }

    if (m_client) {
        m_client->setToolCallOrchestrator(m_orchestrator);

        connect(m_client.get(), &chat::ConversationClient::tokenReceived,
                this, &ToolEnabledChatEntry::tokenReceived);
        connect(m_client.get(), &chat::ConversationClient::responseReceived,
                this, &ToolEnabledChatEntry::onClientResponse);
        connect(m_client.get(), &chat::ConversationClient::errorOccurred,
                this, &ToolEnabledChatEntry::errorOccurred);
    }
}

void ToolEnabledChatEntry::sendUserMessage(const QString &content)
{
    if (!m_client) {
        emit errorOccurred(QStringLiteral("ToolEnabledChatEntry has no bound client"));
        return;
    }

    const QString trimmed = content.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }

    if (m_orchestrator) {
        m_orchestrator->resetSession(m_client->uid(), m_client->activeSessionId());
    }

    const QJsonArray tools = selectAndAdaptToolsForTurn(trimmed);
    m_client->sendUserMessageWithTools(trimmed, tools);
}

void ToolEnabledChatEntry::setToolSelectionLayer(ToolSelectionLayer selectionLayer)
{
    m_selectionLayer = std::move(selectionLayer);
}

void ToolEnabledChatEntry::setToolAdapter(std::unique_ptr<ILlmToolAdapter> adapter)
{
    if (adapter) {
        m_toolAdapter = std::move(adapter);
    }
}

void ToolEnabledChatEntry::setExecutionLayer(const std::shared_ptr<runtime::ToolExecutionLayer> &executionLayer)
{
    if (executionLayer) {
        m_executionLayer = executionLayer;
        if (m_orchestrator) {
            m_orchestrator->setExecutionLayer(executionLayer);
        }
    }
}

void ToolEnabledChatEntry::setClientPolicyRepository(
    const std::shared_ptr<runtime::ClientToolPolicyRepository> &policyRepository)
{
    m_clientPolicyRepository = policyRepository;
    if (m_orchestrator) {
        m_orchestrator->setPolicyRepository(policyRepository);
    }
}

void ToolEnabledChatEntry::setMcpClient(const std::shared_ptr<mcp::IMcpClient> &mcpClient)
{
    if (m_executionLayer) {
        m_executionLayer->setMcpClient(mcpClient);
    }
}

void ToolEnabledChatEntry::setMcpServerRegistry(const std::shared_ptr<mcp::McpServerRegistry> &serverRegistry)
{
    if (m_executionLayer) {
        m_executionLayer->setMcpServerRegistry(serverRegistry);
    }
}

void ToolEnabledChatEntry::setTraceContext(const QString &requestId, const QString &traceId)
{
    m_requestId = requestId;
    m_traceId = traceId;
}

QList<runtime::ToolExecutionResult> ToolEnabledChatEntry::executeToolCalls(
    const QList<runtime::ToolCallRequest> &requests)
{
    if (!m_client) {
        emit errorOccurred(QStringLiteral("ToolEnabledChatEntry has no bound client"));
        return {};
    }

    if (!m_executionLayer) {
        emit errorOccurred(QStringLiteral("ToolEnabledChatEntry has no execution layer"));
        return {};
    }

    runtime::ClientToolPolicy policy;
    policy.clientId = m_client->uid();

    if (m_clientPolicyRepository) {
        const std::optional<runtime::ClientToolPolicy> loaded =
            m_clientPolicyRepository->loadPolicy(m_client->uid(), nullptr);
        if (loaded.has_value()) {
            policy = *loaded;
        }
    }

    return m_executionLayer->executeBatch(requests, buildExecutionContext(), policy);
}

QJsonArray ToolEnabledChatEntry::selectAndAdaptToolsForTurn(const QString &content) const
{
    if (!m_client || !m_toolRegistry || !m_toolAdapter) {
        return QJsonArray();
    }

    QList<LlmToolDefinition> available = m_toolRegistry->enabledTools();

    if (m_clientPolicyRepository) {
        const std::optional<runtime::ClientToolPolicy> policy =
            m_clientPolicyRepository->loadPolicy(m_client->uid(), nullptr);

        if (policy.has_value()) {
            QList<LlmToolDefinition> filtered;
            runtime::ToolExecutionPolicy execPolicy;
            for (const LlmToolDefinition &tool : available) {
                if (execPolicy.isToolAllowed(tool.toolId, *policy)) {
                    filtered.append(tool);
                }
            }
            available = filtered;
        }
    }

    QVector<LlmMessage> historyWindow = m_client->history();
    LlmMessage turnUserMessage;
    turnUserMessage.role = QStringLiteral("user");
    turnUserMessage.content = content;
    historyWindow.append(turnUserMessage);

    const QList<LlmToolDefinition> candidates =
        m_selectionLayer.selectTools(m_client->profile(), historyWindow, available);
    if (candidates.isEmpty()) {
        return QJsonArray();
    }

    return m_toolAdapter->adaptTools(candidates, m_client->config().providerName);
}

runtime::ToolExecutionContext ToolEnabledChatEntry::buildExecutionContext() const
{
    runtime::ToolExecutionContext context;
    if (!m_client) {
        return context;
    }

    context.clientId = m_client->uid();
    context.sessionId = m_client->activeSessionId();
    context.profile = m_client->profile();
    context.llmConfig = m_client->config();
    context.historyWindow = m_client->history();
    context.requestId = m_requestId;
    context.traceId = m_traceId;
    context.extra.insert(QStringLiteral("requestTimestamp"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    return context;
}

void ToolEnabledChatEntry::onClientResponse(const LlmResponse &response)
{
    const QString text = response.assistantMessage.content.isEmpty() ? response.text : response.assistantMessage.content;
    emit completed(text);
}

} // namespace qtllm::tools

