#include "providerprotocoladapters.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStringList>

namespace qtllm::tools::protocol {

namespace {

QList<runtime::ToolCallRequest> parseOpenAIStyleJson(const QString &assistantText)
{
    QList<runtime::ToolCallRequest> requests;
    const QJsonDocument doc = QJsonDocument::fromJson(assistantText.toUtf8());
    if (!doc.isObject()) {
        return requests;
    }

    const QJsonArray toolCalls = doc.object().value(QStringLiteral("tool_calls")).toArray();
    for (const QJsonValue &value : toolCalls) {
        const QJsonObject item = value.toObject();
        const QJsonObject function = item.value(QStringLiteral("function")).toObject();
        runtime::ToolCallRequest request;
        request.callId = item.value(QStringLiteral("id")).toString();
        request.toolId = function.value(QStringLiteral("name")).toString();

        const QString argumentsText = function.value(QStringLiteral("arguments")).toString();
        const QJsonDocument argsDoc = QJsonDocument::fromJson(argumentsText.toUtf8());
        if (argsDoc.isObject()) {
            request.arguments = argsDoc.object();
        }

        if (!request.toolId.trimmed().isEmpty()) {
            if (request.callId.isEmpty()) {
                request.callId = QStringLiteral("tool-") + QString::number(requests.size() + 1);
            }
            requests.append(request);
        }
    }

    return requests;
}

QList<runtime::ToolCallRequest> parseMarkerStyle(const QString &assistantText)
{
    QList<runtime::ToolCallRequest> requests;

    // Marker format: [[tool:tool_id]] or [[tool:tool_id {"k":"v"}]]
    const QRegularExpression re(QStringLiteral("\\[\\[tool:([a-zA-Z0-9_\\-]+)(\\s+\\{.*?\\})?\\]\\]"),
                                QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = re.globalMatch(assistantText);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        runtime::ToolCallRequest request;
        request.toolId = match.captured(1).trimmed();
        request.callId = QStringLiteral("tool-") + QString::number(requests.size() + 1);

        const QString argsText = match.captured(2).trimmed();
        if (!argsText.isEmpty()) {
            const QJsonDocument argsDoc = QJsonDocument::fromJson(argsText.toUtf8());
            if (argsDoc.isObject()) {
                request.arguments = argsDoc.object();
            }
        }

        if (!request.toolId.isEmpty()) {
            requests.append(request);
        }
    }

    return requests;
}

QString buildEmptyToolResultPrompt(const QString &assistantText,
                                   const QList<runtime::ToolExecutionResult> &results,
                                   const QString &adapterId)
{
    Q_UNUSED(assistantText)

    QJsonArray array;
    for (const runtime::ToolExecutionResult &result : results) {
        QJsonObject item;
        item.insert(QStringLiteral("call_id"), result.callId);
        item.insert(QStringLiteral("tool"), result.toolId);
        item.insert(QStringLiteral("content"), QStringLiteral(""));
        item.insert(QStringLiteral("success"), result.success);
        array.append(item);
    }

    QStringList lines;
    lines.append(QStringLiteral("[tool-loop]"));
    lines.append(QStringLiteral("adapter=") + adapterId);
    lines.append(QStringLiteral("tool_results=") + QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact)));
    lines.append(QStringLiteral("Continue by integrating the tool results above."));
    return lines.join(QStringLiteral("\n"));
}

} // namespace

QString OpenAIToolCallProtocolAdapter::adapterId() const
{
    return QStringLiteral("openai");
}

QList<runtime::ToolCallRequest> OpenAIToolCallProtocolAdapter::parseToolCalls(const QString &assistantText) const
{
    QList<runtime::ToolCallRequest> requests = parseOpenAIStyleJson(assistantText);
    if (!requests.isEmpty()) {
        return requests;
    }
    return parseMarkerStyle(assistantText);
}

QString OpenAIToolCallProtocolAdapter::buildFollowUpPrompt(const QString &assistantText,
                                                           const QList<runtime::ToolExecutionResult> &results) const
{
    return buildEmptyToolResultPrompt(assistantText, results, adapterId());
}

QString OllamaToolCallProtocolAdapter::adapterId() const
{
    return QStringLiteral("ollama");
}

QList<runtime::ToolCallRequest> OllamaToolCallProtocolAdapter::parseToolCalls(const QString &assistantText) const
{
    return parseMarkerStyle(assistantText);
}

QString OllamaToolCallProtocolAdapter::buildFollowUpPrompt(const QString &assistantText,
                                                           const QList<runtime::ToolExecutionResult> &results) const
{
    return buildEmptyToolResultPrompt(assistantText, results, adapterId());
}

QString VllmToolCallProtocolAdapter::adapterId() const
{
    return QStringLiteral("vllm");
}

QList<runtime::ToolCallRequest> VllmToolCallProtocolAdapter::parseToolCalls(const QString &assistantText) const
{
    QList<runtime::ToolCallRequest> requests = parseOpenAIStyleJson(assistantText);
    if (!requests.isEmpty()) {
        return requests;
    }
    return parseMarkerStyle(assistantText);
}

QString VllmToolCallProtocolAdapter::buildFollowUpPrompt(const QString &assistantText,
                                                         const QList<runtime::ToolExecutionResult> &results) const
{
    return buildEmptyToolResultPrompt(assistantText, results, adapterId());
}

QString GenericToolCallProtocolAdapter::adapterId() const
{
    return QStringLiteral("generic");
}

QList<runtime::ToolCallRequest> GenericToolCallProtocolAdapter::parseToolCalls(const QString &assistantText) const
{
    return parseMarkerStyle(assistantText);
}

QString GenericToolCallProtocolAdapter::buildFollowUpPrompt(const QString &assistantText,
                                                            const QList<runtime::ToolExecutionResult> &results) const
{
    return buildEmptyToolResultPrompt(assistantText, results, adapterId());
}

} // namespace qtllm::tools::protocol

