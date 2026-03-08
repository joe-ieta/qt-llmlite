#include "qtllmclient.h"

#include "../network/httpexecutor.h"
#include "../providers/illmprovider.h"
#include "../providers/providerfactory.h"
#include "../streaming/streamchunkparser.h"
#include "../tools/runtime/toolcallorchestrator.h"
#include "../tools/runtime/toolruntime_types.h"

#include <QDateTime>

namespace qtllm {

QtLLMClient::QtLLMClient(QObject *parent)
    : QObject(parent)
    , m_executor(new HttpExecutor(this))
    , m_streamParser(std::make_unique<StreamChunkParser>())
    , m_toolOrchestrator(std::make_shared<tools::runtime::ToolCallOrchestrator>())
{
    wireExecutor();
}

QtLLMClient::~QtLLMClient() = default;

void QtLLMClient::setConfig(const LlmConfig &config)
{
    m_config = config;
    if (m_provider) {
        m_provider->setConfig(m_config);
    }
}

void QtLLMClient::setProvider(std::unique_ptr<ILLMProvider> provider)
{
    m_provider = std::move(provider);
    if (m_provider) {
        m_provider->setConfig(m_config);
    }
}

bool QtLLMClient::setProviderByName(const QString &providerName)
{
    std::unique_ptr<ILLMProvider> provider = ProviderFactory::create(providerName);
    if (!provider) {
        emit errorOccurred(QStringLiteral("Unsupported provider: ") + providerName);
        return false;
    }

    setProvider(std::move(provider));
    return true;
}

void QtLLMClient::setToolCallOrchestrator(
    const std::shared_ptr<tools::runtime::ToolCallOrchestrator> &orchestrator)
{
    if (orchestrator) {
        m_toolOrchestrator = orchestrator;
    }
}

void QtLLMClient::setToolLoopContext(const QString &clientId, const QString &sessionId)
{
    m_toolLoopClientId = clientId.trimmed();
    m_toolLoopSessionId = sessionId.trimmed();
}

void QtLLMClient::sendPrompt(const QString &prompt)
{
    LlmRequest request;
    request.model = m_config.model;
    request.stream = m_config.stream;
    request.messages.append({QStringLiteral("user"), prompt});
    sendRequest(request);
}

void QtLLMClient::sendRequest(const LlmRequest &request)
{
    if (!m_provider) {
        if (!m_config.providerName.isEmpty()) {
            if (!setProviderByName(m_config.providerName)) {
                return;
            }
        } else {
            emit errorOccurred(QStringLiteral("No provider configured"));
            return;
        }
    }

    dispatchRequest(request);
}

void QtLLMClient::cancelCurrentRequest()
{
    m_executor->cancel();
}

void QtLLMClient::dispatchRequest(const LlmRequest &request)
{
    m_accumulatedText.clear();
    m_streamParser->clear();

    LlmRequest resolved = request;
    if (resolved.model.trimmed().isEmpty()) {
        resolved.model = m_config.model;
    }
    m_activeRequest = resolved;

    const QNetworkRequest networkRequest = m_provider->buildRequest(resolved);
    const QByteArray payload = m_provider->buildPayload(resolved);

    HttpRequestOptions options;
    options.timeoutMs = m_config.timeoutMs;
    options.maxRetries = m_config.maxRetries;
    options.retryDelayMs = m_config.retryDelayMs;

    m_executor->post(networkRequest, payload, options);
}

void QtLLMClient::wireExecutor()
{
    connect(m_executor, &HttpExecutor::dataReceived, this, [this](const QByteArray &chunk) {
        if (!m_provider || !m_activeRequest.stream) {
            return;
        }

        const QStringList lines = m_streamParser->append(chunk);
        for (const QString &line : lines) {
            const QList<QString> tokens = m_provider->parseStreamTokens(line.toUtf8());
            for (const QString &token : tokens) {
                m_accumulatedText += token;
                emit tokenReceived(token);
            }
        }
    });

    connect(m_executor, &HttpExecutor::requestFinished, this, [this](const QByteArray &data) {
        if (!m_provider) {
            return;
        }

        if (m_activeRequest.stream) {
            const QString pendingLine = m_streamParser->takePendingLine();
            if (!pendingLine.isEmpty()) {
                const QList<QString> tokens = m_provider->parseStreamTokens(pendingLine.toUtf8());
                for (const QString &token : tokens) {
                    m_accumulatedText += token;
                    emit tokenReceived(token);
                }
            }
        }

        LlmResponse response = m_provider->parseResponse(data);

        if (!m_accumulatedText.isEmpty() && response.assistantMessage.content.isEmpty()) {
            response.assistantMessage.role = QStringLiteral("assistant");
            response.assistantMessage.content = m_accumulatedText;
            response.text = m_accumulatedText;
            response.success = true;
        }

        if (!response.success) {
            emit errorOccurred(response.errorMessage);
            m_streamParser->clear();
            return;
        }

        QString finalText = response.assistantMessage.content;
        if (finalText.isEmpty()) {
            finalText = response.text;
        }

        if (m_toolOrchestrator && !m_toolLoopClientId.isEmpty() && !m_toolLoopSessionId.isEmpty()) {
            tools::runtime::ToolExecutionContext context;
            context.clientId = m_toolLoopClientId;
            context.sessionId = m_toolLoopSessionId;
            context.llmConfig = m_config;
            context.historyWindow = m_activeRequest.messages;
            context.extra.insert(QStringLiteral("requestTimestamp"),
                                 QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

            const tools::runtime::ToolLoopOutcome outcome = m_toolOrchestrator->processAssistantResponse(
                m_activeRequest.model,
                m_config.modelVendor,
                m_config.providerName,
                response,
                context);

            if (outcome.terminatedByFailureGuard) {
                emit errorOccurred(QStringLiteral("Tool loop stopped after repeated failures"));
                emit responseReceived(response);
                emit completed(finalText);
                m_streamParser->clear();
                return;
            }

            if (outcome.hasFollowUpPrompt && !outcome.followUpPrompt.trimmed().isEmpty()) {
                LlmMessage assistant = response.assistantMessage;
                if (assistant.role.trimmed().isEmpty()) {
                    assistant.role = QStringLiteral("assistant");
                }
                if (assistant.content.isEmpty()) {
                    assistant.content = finalText;
                }

                LlmMessage user;
                user.role = QStringLiteral("user");
                user.content = outcome.followUpPrompt;

                m_activeRequest.messages.append(assistant);
                m_activeRequest.messages.append(user);
                dispatchRequest(m_activeRequest);
                return;
            }
        }

        emit responseReceived(response);
        emit completed(finalText);
        m_streamParser->clear();
    });

    connect(m_executor, &HttpExecutor::errorOccurred, this, [this](const QString &message) {
        m_streamParser->clear();
        emit errorOccurred(message);
    });
}

} // namespace qtllm
