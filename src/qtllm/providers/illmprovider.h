#pragma once

#include "../core/llmconfig.h"
#include "../core/llmtypes.h"

#include <QByteArray>
#include <QList>
#include <QNetworkRequest>
#include <QString>

namespace qtllm {

class ILLMProvider
{
public:
    virtual ~ILLMProvider() = default;

    virtual QString name() const = 0;
    virtual void setConfig(const LlmConfig &config) = 0;

    virtual QNetworkRequest buildRequest(const LlmRequest &request) const = 0;
    virtual QByteArray buildPayload(const LlmRequest &request) const = 0;
    virtual LlmResponse parseResponse(const QByteArray &data) const = 0;

    virtual QList<QString> parseStreamTokens(const QByteArray &chunk) const = 0;

    virtual QList<LlmStreamDelta> parseStreamDeltas(const QByteArray &chunk) const
    {
        QList<LlmStreamDelta> deltas;
        const QList<QString> tokens = parseStreamTokens(chunk);
        for (const QString &token : tokens) {
            LlmStreamDelta delta;
            delta.channel = QStringLiteral("content");
            delta.text = token;
            deltas.append(delta);
        }
        return deltas;
    }

    virtual bool supportsStructuredToolCalls() const
    {
        return false;
    }
};

} // namespace qtllm
