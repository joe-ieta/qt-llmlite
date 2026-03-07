#include "qtllmclient.h"

#include "../network/httpexecutor.h"
#include "../providers/illmprovider.h"
#include "../providers/providerfactory.h"
#include "../streaming/streamchunkparser.h"

namespace qtllm {

QtLLMClient::QtLLMClient(QObject *parent)
    : QObject(parent)
    , m_executor(new HttpExecutor(this))
    , m_streamParser(std::make_unique<StreamChunkParser>())
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

void QtLLMClient::sendPrompt(const QString &prompt)
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

    m_accumulatedText.clear();
    m_streamParser->clear();

    LlmRequest request;
    request.model = m_config.model;
    request.stream = m_config.stream;
    request.messages.append({QStringLiteral("user"), prompt});

    const QNetworkRequest networkRequest = m_provider->buildRequest(request);
    const QByteArray payload = m_provider->buildPayload(request);

    HttpRequestOptions options;
    options.timeoutMs = m_config.timeoutMs;
    options.maxRetries = m_config.maxRetries;
    options.retryDelayMs = m_config.retryDelayMs;

    m_executor->post(networkRequest, payload, options);
}

void QtLLMClient::cancelCurrentRequest()
{
    m_executor->cancel();
}

void QtLLMClient::wireExecutor()
{
    connect(m_executor, &HttpExecutor::dataReceived, this, [this](const QByteArray &chunk) {
        if (!m_provider || !m_config.stream) {
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

        if (m_config.stream) {
            const QString pendingLine = m_streamParser->takePendingLine();
            if (!pendingLine.isEmpty()) {
                const QList<QString> tokens = m_provider->parseStreamTokens(pendingLine.toUtf8());
                for (const QString &token : tokens) {
                    m_accumulatedText += token;
                    emit tokenReceived(token);
                }
            }

            if (!m_accumulatedText.isEmpty()) {
                emit completed(m_accumulatedText);
                m_streamParser->clear();
                return;
            }
        }

        const LlmResponse response = m_provider->parseResponse(data);
        if (!response.success) {
            emit errorOccurred(response.errorMessage);
            m_streamParser->clear();
            return;
        }

        emit completed(response.text);
        m_streamParser->clear();
    });

    connect(m_executor, &HttpExecutor::errorOccurred, this, [this](const QString &message) {
        m_streamParser->clear();
        emit errorOccurred(message);
    });
}

} // namespace qtllm
