#pragma once

#include <QString>
#include <QVector>

namespace qtllm {

struct LlmMessage
{
    QString role;
    QString content;
};

struct LlmRequest
{
    QVector<LlmMessage> messages;
    QString model;
    bool stream = true;
};

struct LlmResponse
{
    QString text;
    bool success = false;
    QString errorMessage;
};

} // namespace qtllm
