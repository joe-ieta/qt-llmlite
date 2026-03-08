#include "toolenabledchatentry.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QTimeZone>
#include <optional>

namespace qtllm::tools {

namespace {

bool containsAny(const QString &content, const QStringList &terms)
{
    const QString lowered = content.toLower();
    for (const QString &term : terms) {
        if (lowered.contains(term.toLower())) {
            return true;
        }
    }
    return false;
}

QJsonObject inferLocationFromProfile(const profile::ClientProfile &profile)
{
    QJsonObject args;

    for (const QString &tag : profile.preferenceTags) {
        const QString t = tag.trimmed();
        if (t.startsWith(QStringLiteral("lat="), Qt::CaseInsensitive)) {
            bool ok = false;
            const double lat = t.mid(4).toDouble(&ok);
            if (ok) {
                args.insert(QStringLiteral("latitude"), lat);
            }
        }

        if (t.startsWith(QStringLiteral("lon="), Qt::CaseInsensitive)
            || t.startsWith(QStringLiteral("lng="), Qt::CaseInsensitive)) {
            const int pos = t.indexOf('=');
            bool ok = false;
            const double lon = t.mid(pos + 1).toDouble(&ok);
            if (ok) {
                args.insert(QStringLiteral("longitude"), lon);
            }
        }

        if (t.startsWith(QStringLiteral("timezone="), Qt::CaseInsensitive)) {
            args.insert(QStringLiteral("timezone"), t.mid(QStringLiteral("timezone=").size()));
        }
    }

    return args;
}

} // namespace

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

    const QList<runtime::ToolCallRequest> requests = planBuiltInToolCalls(trimmed);
    const QList<runtime::ToolExecutionResult> results = executeToolCalls(requests);

    QString prompt = buildToolAwareMessage(trimmed);
    if (!results.isEmpty()) {
        QStringList lines;
        lines.append(prompt);
        lines.append(QStringLiteral("[tool-results]"));
        lines.append(QString::fromUtf8(QJsonDocument(toToolResultJson(results)).toJson(QJsonDocument::Compact)));
        prompt = lines.join(QStringLiteral("\n\n"));
    }

    m_client->sendUserMessage(prompt);
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

QString ToolEnabledChatEntry::buildToolAwareMessage(const QString &content) const
{
    if (!m_toolRegistry || !m_toolAdapter) {
        return content;
    }

    QList<LlmToolDefinition> available = m_toolRegistry->enabledTools();

    if (m_clientPolicyRepository && m_client) {
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

    const QList<LlmToolDefinition> candidates = m_selectionLayer.selectTools(
        m_client->profile(), m_client->history(), available);

    if (candidates.isEmpty()) {
        return content;
    }

    const QString providerName = m_client->config().providerName;
    const QJsonArray adapted = m_toolAdapter->adaptTools(candidates, providerName);

    QStringList lines;
    lines.append(QStringLiteral("[tool-selection]"));
    lines.append(QStringLiteral("The following tools are available for this turn."));
    lines.append(QString::fromUtf8(QJsonDocument(adapted).toJson(QJsonDocument::Compact)));
    lines.append(QStringLiteral("[user-message]"));
    lines.append(content);
    return lines.join(QStringLiteral("\n\n"));
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

QList<runtime::ToolCallRequest> ToolEnabledChatEntry::planBuiltInToolCalls(const QString &content) const
{
    QList<runtime::ToolCallRequest> requests;
    if (!m_client) {
        return requests;
    }

    const bool askTime = containsAny(content,
                                     QStringList({QStringLiteral("time"), QStringLiteral("date"), QStringLiteral("now")}));
    const bool askWeather = containsAny(content,
                                        QStringList({QStringLiteral("weather"), QStringLiteral("temperature")}));

    if (askTime) {
        runtime::ToolCallRequest call;
        call.callId = QStringLiteral("time-") + QString::number(QDateTime::currentMSecsSinceEpoch());
        call.toolId = QStringLiteral("current_time");
        call.arguments.insert(QStringLiteral("timezone"), QString::fromUtf8(QTimeZone::systemTimeZoneId()));
        requests.append(call);
    }

    if (askWeather) {
        runtime::ToolCallRequest call;
        call.callId = QStringLiteral("weather-") + QString::number(QDateTime::currentMSecsSinceEpoch());
        call.toolId = QStringLiteral("current_weather");
        call.arguments = inferLocationFromProfile(m_client->profile());
        requests.append(call);
    }

    return requests;
}

QJsonArray ToolEnabledChatEntry::toToolResultJson(const QList<runtime::ToolExecutionResult> &results) const
{
    QJsonArray array;
    for (const runtime::ToolExecutionResult &result : results) {
        QJsonObject item;
        item.insert(QStringLiteral("callId"), result.callId);
        item.insert(QStringLiteral("toolId"), result.toolId);
        item.insert(QStringLiteral("success"), result.success);
        item.insert(QStringLiteral("durationMs"), static_cast<double>(result.durationMs));
        if (result.success) {
            item.insert(QStringLiteral("output"), result.output);
        } else {
            item.insert(QStringLiteral("errorCode"), result.errorCode);
            item.insert(QStringLiteral("errorMessage"), result.errorMessage);
            item.insert(QStringLiteral("retryable"), result.retryable);
        }
        array.append(item);
    }
    return array;
}

void ToolEnabledChatEntry::onClientResponse(const LlmResponse &response)
{
    const QString text = response.assistantMessage.content.isEmpty() ? response.text : response.assistantMessage.content;
    emit completed(text);
}
} // namespace qtllm::tools
