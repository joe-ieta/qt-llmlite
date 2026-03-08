#pragma once

#include "illmprovider.h"

namespace qtllm {

class OpenAICompatibleProvider : public ILLMProvider
{
public:
    QString name() const override;
    void setConfig(const LlmConfig &config) override;

    QNetworkRequest buildRequest(const LlmRequest &request) const override;
    QByteArray buildPayload(const LlmRequest &request) const override;
    LlmResponse parseResponse(const QByteArray &data) const override;
    QList<QString> parseStreamTokens(const QByteArray &chunk) const override;
    bool supportsStructuredToolCalls() const override;

private:
    LlmConfig m_config;
};

} // namespace qtllm
