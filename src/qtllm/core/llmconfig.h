#pragma once

#include <QString>

namespace qtllm {

struct LlmConfig
{
    QString providerName;
    QString baseUrl;
    QString apiKey;
    QString model;
    QString modelVendor; // Optional explicit vendor/family hint for protocol routing.
    bool stream = true;

    int timeoutMs = 60000;
    int maxRetries = 0;
    int retryDelayMs = 400;
};

} // namespace qtllm
