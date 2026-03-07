#include "openaicompatibleprovider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

QString OpenAICompatibleProvider::name() const
{
    return QStringLiteral("openai-compatible");
}

void OpenAICompatibleProvider::setConfig(const LlmConfig &config)
{
    m_config = config;
}

QNetworkRequest OpenAICompatibleProvider::buildRequest(const LlmRequest &request) const
{
    Q_UNUSED(request)

    QUrl url(m_config.baseUrl);
    if (!url.path().endsWith(QStringLiteral("/chat/completions"))) {
        QString path = url.path();
        if (!path.endsWith('/')) {
            path += '/';
        }
        path += QStringLiteral("chat/completions");
        url.setPath(path);
    }

    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    if (!m_config.apiKey.isEmpty()) {
        const QByteArray bearer = "Bearer " + m_config.apiKey.toUtf8();
        networkRequest.setRawHeader("Authorization", bearer);
    }

    return networkRequest;
}

QByteArray OpenAICompatibleProvider::buildPayload(const LlmRequest &request) const
{
    QJsonArray messages;
    for (const LlmMessage &message : request.messages) {
        QJsonObject obj;
        obj.insert(QStringLiteral("role"), message.role);
        obj.insert(QStringLiteral("content"), message.content);
        messages.append(obj);
    }

    QJsonObject root;
    root.insert(QStringLiteral("model"), request.model.isEmpty() ? m_config.model : request.model);
    root.insert(QStringLiteral("stream"), request.stream);
    root.insert(QStringLiteral("messages"), messages);

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

LlmResponse OpenAICompatibleProvider::parseResponse(const QByteArray &data) const
{
    LlmResponse response;

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        response.errorMessage = QStringLiteral("Invalid JSON response");
        return response;
    }

    const QJsonObject root = doc.object();
    const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        response.errorMessage = QStringLiteral("No choices found in response");
        return response;
    }

    const QJsonObject firstChoice = choices.first().toObject();
    const QJsonObject message = firstChoice.value(QStringLiteral("message")).toObject();
    response.text = message.value(QStringLiteral("content")).toString();
    response.success = !response.text.isEmpty();

    if (!response.success && response.text.isEmpty()) {
        response.errorMessage = QStringLiteral("Empty content in response");
    }

    return response;
}

QList<QString> OpenAICompatibleProvider::parseStreamTokens(const QByteArray &chunk) const
{
    QList<QString> tokens;

    const QList<QByteArray> lines = chunk.split('\n');
    for (const QByteArray &line : lines) {
        const QByteArray trimmed = line.trimmed();
        if (!trimmed.startsWith("data:")) {
            continue;
        }

        const QByteArray payload = trimmed.mid(5).trimmed();
        if (payload == "[DONE]") {
            continue;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(payload);
        if (!doc.isObject()) {
            continue;
        }

        const QJsonArray choices = doc.object().value(QStringLiteral("choices")).toArray();
        if (choices.isEmpty()) {
            continue;
        }

        const QJsonObject delta = choices.first().toObject().value(QStringLiteral("delta")).toObject();
        const QString content = delta.value(QStringLiteral("content")).toString();
        if (!content.isEmpty()) {
            tokens.append(content);
        }
    }

    return tokens;
}
