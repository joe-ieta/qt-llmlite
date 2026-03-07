#include "qtllmclient.h"

#include "../network/httpexecutor.h"
#include "../providers/illmprovider.h"

QtLLMClient::QtLLMClient(QObject *parent)
    : QObject(parent)
    , m_executor(new HttpExecutor(this))
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

void QtLLMClient::sendPrompt(const QString &prompt)
{
    if (!m_provider) {
        emit errorOccurred(QStringLiteral("No provider configured"));
        return;
    }

    m_accumulatedText.clear();

    LlmRequest request;
    request.model = m_config.model;
    request.stream = m_config.stream;
    request.messages.append({QStringLiteral("user"), prompt});

    const QNetworkRequest networkRequest = m_provider->buildRequest(request);
    const QByteArray payload = m_provider->buildPayload(request);
    m_executor->post(networkRequest, payload);
}

void QtLLMClient::wireExecutor()
{
    connect(m_executor, &HttpExecutor::dataReceived, this, [this](const QByteArray &chunk) {
        if (!m_provider || !m_config.stream) {
            return;
        }

        const QList<QString> tokens = m_provider->parseStreamTokens(chunk);
        for (const QString &token : tokens) {
            m_accumulatedText += token;
            emit tokenReceived(token);
        }
    });

    connect(m_executor, &HttpExecutor::requestFinished, this, [this](const QByteArray &data) {
        if (!m_provider) {
            return;
        }

        if (m_config.stream && !m_accumulatedText.isEmpty()) {
            emit completed(m_accumulatedText);
            return;
        }

        const LlmResponse response = m_provider->parseResponse(data);
        if (!response.success) {
            emit errorOccurred(response.errorMessage);
            return;
        }

        emit completed(response.text);
    });

    connect(m_executor, &HttpExecutor::errorOccurred, this, &QtLLMClient::errorOccurred);
}
