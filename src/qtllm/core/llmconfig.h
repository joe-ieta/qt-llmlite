#pragma once

#include <QString>

struct LlmConfig
{
    QString providerName;
    QString baseUrl;
    QString apiKey;
    QString model;
    bool stream = true;

    int timeoutMs = 60000;
    int maxRetries = 0;
    int retryDelayMs = 400;
};