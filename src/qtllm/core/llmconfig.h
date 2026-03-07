#pragma once

#include <QString>

struct LlmConfig
{
    QString providerName;
    QString baseUrl;
    QString apiKey;
    QString model;
    bool stream = true;
};
