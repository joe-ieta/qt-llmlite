#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

namespace qtllm {

struct LlmToolCall
{
    QString id;
    QString name;
    QJsonObject arguments;
    QString type = QStringLiteral("function");
};

struct LlmMessage
{
    QString role;
    QString content;

    // Optional fields for tool-calling protocol.
    QString name;
    QString toolCallId;
    QVector<LlmToolCall> toolCalls;
};

struct LlmRequest
{
    QVector<LlmMessage> messages;
    QString model;
    bool stream = true;

    // OpenAI-compatible tools schema array.
    QJsonArray tools;
};

struct LlmResponse
{
    QString text;
    bool success = false;
    QString errorMessage;

    // Structured assistant response fields.
    LlmMessage assistantMessage;
    QString finishReason;
};

struct LlmStreamDelta
{
    QString channel = QStringLiteral("content");
    QString text;
};

} // namespace qtllm
