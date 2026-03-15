#include "openaiprovider.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QRegularExpression>
#include <QUrl>

namespace qtllm {

namespace {

struct ToolLoopResultItem
{
    QString callId;
    QString toolId;
    QString output;
    bool success = false;
    QString errorCode;
};

struct PendingFunctionCall
{
    QString callId;
    QString name;
    QString argumentsText;
};

QString compactJson(const QJsonObject &object)
{
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

QJsonObject buildMessageItem(const QString &role,
                             const QString &contentType,
                             const QString &text)
{
    QJsonObject item;
    item.insert(QStringLiteral("type"), QStringLiteral("message"));
    item.insert(QStringLiteral("role"), role);

    QJsonArray content;
    QJsonObject textItem;
    textItem.insert(QStringLiteral("type"), contentType);
    textItem.insert(QStringLiteral("text"), text);
    content.append(textItem);
    item.insert(QStringLiteral("content"), content);
    return item;
}

QJsonObject buildFunctionCallItem(const LlmToolCall &call)
{
    QJsonObject item;
    item.insert(QStringLiteral("type"), QStringLiteral("function_call"));
    if (!call.id.trimmed().isEmpty()) {
        item.insert(QStringLiteral("call_id"), call.id);
    }
    item.insert(QStringLiteral("name"), call.name);
    item.insert(QStringLiteral("arguments"), QString::fromUtf8(QJsonDocument(call.arguments).toJson(QJsonDocument::Compact)));
    return item;
}

QString extractToolResultsJson(const QString &content)
{
    if (!content.startsWith(QStringLiteral("[tool-loop]"))) {
        return QString();
    }

    const QStringList lines = content.split('\n');
    for (const QString &line : lines) {
        if (line.startsWith(QStringLiteral("tool_results="))) {
            return line.mid(QStringLiteral("tool_results=").size()).trimmed();
        }
    }

    return QString();
}

QList<ToolLoopResultItem> parseToolLoopResults(const QString &content)
{
    QList<ToolLoopResultItem> items;
    const QString payload = extractToolResultsJson(content);
    if (payload.isEmpty()) {
        return items;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(payload.toUtf8());
    if (!doc.isArray()) {
        return items;
    }

    const QJsonArray array = doc.array();
    for (const QJsonValue &value : array) {
        const QJsonObject object = value.toObject();
        ToolLoopResultItem item;
        item.callId = object.value(QStringLiteral("call_id")).toString();
        item.toolId = object.value(QStringLiteral("tool")).toString();
        item.output = object.value(QStringLiteral("output")).toString(object.value(QStringLiteral("content")).toString());
        item.success = object.value(QStringLiteral("success")).toBool(false);
        item.errorCode = object.value(QStringLiteral("error_code")).toString();
        if (!item.callId.trimmed().isEmpty()) {
            items.append(item);
        }
    }

    return items;
}

QJsonArray toResponseInput(const QVector<LlmMessage> &messages, QString *instructions)
{
    QJsonArray input;
    QStringList instructionBlocks;

    for (const LlmMessage &message : messages) {
        const QString role = message.role.trimmed().toLower();

        if (role == QStringLiteral("system") || role == QStringLiteral("developer")) {
            if (!message.content.trimmed().isEmpty()) {
                instructionBlocks.append(message.content);
            }
            continue;
        }

        if (role == QStringLiteral("tool")) {
            QJsonObject item;
            item.insert(QStringLiteral("type"), QStringLiteral("function_call_output"));
            item.insert(QStringLiteral("call_id"), message.toolCallId);
            item.insert(QStringLiteral("output"), message.content);
            input.append(item);
            continue;
        }

        if (role == QStringLiteral("user")) {
            const QList<ToolLoopResultItem> toolResults = parseToolLoopResults(message.content);
            if (!toolResults.isEmpty()) {
                for (const ToolLoopResultItem &result : toolResults) {
                    QJsonObject item;
                    item.insert(QStringLiteral("type"), QStringLiteral("function_call_output"));
                    item.insert(QStringLiteral("call_id"), result.callId);
                    item.insert(QStringLiteral("output"), result.output);
                    input.append(item);
                }
                continue;
            }
        }

        if (!message.content.trimmed().isEmpty()) {
            const QString contentType = role == QStringLiteral("assistant")
                ? QStringLiteral("output_text")
                : QStringLiteral("input_text");
            input.append(buildMessageItem(role, contentType, message.content));
        }

        if (role == QStringLiteral("assistant")) {
            for (const LlmToolCall &call : message.toolCalls) {
                if (!call.name.trimmed().isEmpty()) {
                    input.append(buildFunctionCallItem(call));
                }
            }
        }
    }

    if (instructions) {
        *instructions = instructionBlocks.join(QStringLiteral("\n\n"));
    }

    return input;
}

QJsonValue sanitizeSchemaValue(const QJsonValue &value);

QJsonArray sanitizeSchemaArray(const QJsonArray &array)
{
    QJsonArray sanitized;
    for (const QJsonValue &entry : array) {
        sanitized.append(sanitizeSchemaValue(entry));
    }
    return sanitized;
}

QJsonObject sanitizeSchemaObject(const QJsonObject &object)
{
    static const QSet<QString> blockedKeys = {
        QStringLiteral("$schema"),
        QStringLiteral("$id"),
        QStringLiteral("$comment"),
        QStringLiteral("definitions"),
        QStringLiteral("$defs"),
        QStringLiteral("default"),
        QStringLiteral("example"),
        QStringLiteral("examples"),
        QStringLiteral("title"),
        QStringLiteral("deprecated"),
        QStringLiteral("readOnly"),
        QStringLiteral("writeOnly"),
        QStringLiteral("nullable"),
        QStringLiteral("contentEncoding"),
        QStringLiteral("contentMediaType")
    };

    QJsonObject sanitized;
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (blockedKeys.contains(it.key())) {
            continue;
        }
        sanitized.insert(it.key(), sanitizeSchemaValue(it.value()));
    }

    if (sanitized.contains(QStringLiteral("properties"))
        && !sanitized.contains(QStringLiteral("type"))) {
        sanitized.insert(QStringLiteral("type"), QStringLiteral("object"));
    }

    return sanitized;
}

QJsonValue sanitizeSchemaValue(const QJsonValue &value)
{
    if (value.isObject()) {
        return sanitizeSchemaObject(value.toObject());
    }
    if (value.isArray()) {
        return sanitizeSchemaArray(value.toArray());
    }
    return value;
}
QJsonArray toResponseTools(const QJsonArray &openAiTools)
{
    QJsonArray tools;
    for (const QJsonValue &value : openAiTools) {
        const QJsonObject descriptor = value.toObject();
        const QJsonObject function = descriptor.value(QStringLiteral("function")).toObject();
        const QString type = descriptor.value(QStringLiteral("type")).toString();
        if (type != QStringLiteral("function") || function.isEmpty()) {
            continue;
        }

        const QString name = function.value(QStringLiteral("name")).toString();
        if (name.trimmed().isEmpty()) {
            continue;
        }

        QJsonObject tool;
        tool.insert(QStringLiteral("type"), QStringLiteral("function"));
        tool.insert(QStringLiteral("name"), name);
        tool.insert(QStringLiteral("description"), function.value(QStringLiteral("description")).toString());
        tool.insert(QStringLiteral("parameters"), sanitizeSchemaObject(function.value(QStringLiteral("parameters")).toObject()));
        tool.insert(QStringLiteral("strict"), false);
        tools.append(tool);
    }
    return tools;
}

QString collectMessageText(const QJsonArray &content)
{
    QStringList texts;
    for (const QJsonValue &value : content) {
        const QJsonObject part = value.toObject();
        const QString type = part.value(QStringLiteral("type")).toString();
        if (type == QStringLiteral("output_text") || type == QStringLiteral("text") || type == QStringLiteral("input_text")) {
            const QString text = part.value(QStringLiteral("text")).toString();
            if (!text.isEmpty()) {
                texts.append(text);
            }
        }
    }
    return texts.join(QString());
}

LlmResponse parseResponseObject(const QJsonObject &root)
{
    LlmResponse response;
    response.finishReason = root.value(QStringLiteral("status")).toString();

    LlmMessage assistant;
    assistant.role = QStringLiteral("assistant");

    const QJsonArray output = root.value(QStringLiteral("output")).toArray();
    for (const QJsonValue &value : output) {
        const QJsonObject item = value.toObject();
        const QString type = item.value(QStringLiteral("type")).toString();

        if (type == QStringLiteral("message")) {
            const QString text = collectMessageText(item.value(QStringLiteral("content")).toArray());
            if (!text.isEmpty()) {
                assistant.content += text;
            }
            continue;
        }

        if (type == QStringLiteral("function_call")) {
            LlmToolCall call;
            call.id = item.value(QStringLiteral("call_id")).toString(item.value(QStringLiteral("id")).toString());
            call.name = item.value(QStringLiteral("name")).toString();
            const QString argumentsText = item.value(QStringLiteral("arguments")).toString();
            const QJsonDocument argumentsDoc = QJsonDocument::fromJson(argumentsText.toUtf8());
            if (argumentsDoc.isObject()) {
                call.arguments = argumentsDoc.object();
            }
            if (!call.name.trimmed().isEmpty()) {
                assistant.toolCalls.append(call);
            }
        }
    }

    response.assistantMessage = assistant;
    response.text = assistant.content;
    response.success = !assistant.content.isEmpty() || !assistant.toolCalls.isEmpty();
    if (!response.success) {
        response.errorMessage = QStringLiteral("Empty assistant message in OpenAI responses payload");
    }
    return response;
}

QJsonObject parseSseEvent(const QByteArray &chunk)
{
    const QByteArray trimmed = chunk.trimmed();
    if (!trimmed.startsWith("data:")) {
        return QJsonObject();
    }

    const QByteArray payload = trimmed.mid(5).trimmed();
    if (payload.isEmpty() || payload == "[DONE]") {
        return QJsonObject();
    }

    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    return doc.isObject() ? doc.object() : QJsonObject();
}

} // namespace

QString OpenAIProvider::name() const
{
    return QStringLiteral("openai");
}

void OpenAIProvider::setConfig(const LlmConfig &config)
{
    m_config = config;
}

QNetworkRequest OpenAIProvider::buildRequest(const LlmRequest &request) const
{
    Q_UNUSED(request)

    QUrl url(m_config.baseUrl);
    if (!url.path().endsWith(QStringLiteral("/responses"))) {
        QString path = url.path();
        if (!path.endsWith('/')) {
            path += '/';
        }
        path += QStringLiteral("responses");
        url.setPath(path);
    }

    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    if (!m_config.apiKey.isEmpty()) {
        networkRequest.setRawHeader("Authorization", QByteArray("Bearer ") + m_config.apiKey.toUtf8());
    }
    return networkRequest;
}

QByteArray OpenAIProvider::buildPayload(const LlmRequest &request) const
{
    QJsonObject root;
    root.insert(QStringLiteral("model"), request.model.trimmed().isEmpty() ? m_config.model : request.model);
    root.insert(QStringLiteral("stream"), request.stream);
    root.insert(QStringLiteral("store"), false);

    QString instructions;
    const QJsonArray input = toResponseInput(request.messages, &instructions);
    root.insert(QStringLiteral("input"), input);
    if (!instructions.trimmed().isEmpty()) {
        root.insert(QStringLiteral("instructions"), instructions);
    }

    const QJsonArray tools = toResponseTools(request.tools);
    if (!tools.isEmpty()) {
        root.insert(QStringLiteral("tools"), tools);
    }

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

LlmResponse OpenAIProvider::parseResponse(const QByteArray &data) const
{
    const QByteArray trimmed = data.trimmed();
    if (trimmed.startsWith("data:")) {
        QStringList textParts;
        QString finishReason;
        QHash<QString, PendingFunctionCall> calls;
        QJsonObject completedResponse;

        const QList<QByteArray> lines = trimmed.split('\n');
        for (const QByteArray &line : lines) {
            const QJsonObject event = parseSseEvent(line);
            if (event.isEmpty()) {
                continue;
            }

            const QString type = event.value(QStringLiteral("type")).toString();
            if (type == QStringLiteral("response.output_text.delta")) {
                const QString delta = event.value(QStringLiteral("delta")).toString();
                if (!delta.isEmpty()) {
                    textParts.append(delta);
                }
                continue;
            }

            if (type == QStringLiteral("response.function_call_arguments.delta")) {
                const QString callId = event.value(QStringLiteral("call_id")).toString();
                PendingFunctionCall &call = calls[callId];
                call.callId = callId;
                call.name = event.value(QStringLiteral("name")).toString(call.name);
                call.argumentsText += event.value(QStringLiteral("delta")).toString();
                continue;
            }

            if (type == QStringLiteral("response.output_item.added") || type == QStringLiteral("response.output_item.done")) {
                const QJsonObject item = event.value(QStringLiteral("item")).toObject();
                if (item.value(QStringLiteral("type")).toString() == QStringLiteral("function_call")) {
                    const QString callId = item.value(QStringLiteral("call_id")).toString(item.value(QStringLiteral("id")).toString());
                    PendingFunctionCall &call = calls[callId];
                    call.callId = callId;
                    call.name = item.value(QStringLiteral("name")).toString(call.name);
                    const QString argumentsText = item.value(QStringLiteral("arguments")).toString();
                    if (!argumentsText.isEmpty()) {
                        call.argumentsText = argumentsText;
                    }
                }
                continue;
            }

            if (type == QStringLiteral("response.completed")) {
                completedResponse = event.value(QStringLiteral("response")).toObject();
                finishReason = completedResponse.value(QStringLiteral("status")).toString(finishReason);
            }
        }

        LlmResponse response;
        if (!completedResponse.isEmpty()) {
            response = parseResponseObject(completedResponse);
        }

        if (response.assistantMessage.content.isEmpty() && !textParts.isEmpty()) {
            response.assistantMessage.role = QStringLiteral("assistant");
            response.assistantMessage.content = textParts.join(QString());
            response.text = response.assistantMessage.content;
            response.success = true;
        }

        for (auto it = calls.constBegin(); it != calls.constEnd(); ++it) {
            if (it->name.trimmed().isEmpty()) {
                continue;
            }
            bool exists = false;
            for (const LlmToolCall &call : response.assistantMessage.toolCalls) {
                if (call.id == it->callId || call.name == it->name) {
                    exists = true;
                    break;
                }
            }
            if (exists) {
                continue;
            }
            LlmToolCall call;
            call.id = it->callId;
            call.name = it->name;
            const QJsonDocument argsDoc = QJsonDocument::fromJson(it->argumentsText.toUtf8());
            if (argsDoc.isObject()) {
                call.arguments = argsDoc.object();
            }
            response.assistantMessage.toolCalls.append(call);
        }

        if (!finishReason.isEmpty() && response.finishReason.isEmpty()) {
            response.finishReason = finishReason;
        }
        if (!response.success) {
            response.success = !response.assistantMessage.content.isEmpty() || !response.assistantMessage.toolCalls.isEmpty();
        }
        if (!response.success) {
            response.errorMessage = QStringLiteral("Empty assistant message in OpenAI SSE response");
        }
        return response;
    }

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (!doc.isObject()) {
        LlmResponse response;
        response.errorMessage = QStringLiteral("Invalid JSON response: ") + error.errorString();
        return response;
    }

    return parseResponseObject(doc.object());
}

QList<QString> OpenAIProvider::parseStreamTokens(const QByteArray &chunk) const
{
    QList<QString> tokens;
    const QList<LlmStreamDelta> deltas = parseStreamDeltas(chunk);
    for (const LlmStreamDelta &delta : deltas) {
        if (delta.channel == QStringLiteral("content")) {
            tokens.append(delta.text);
        }
    }
    return tokens;
}

QList<LlmStreamDelta> OpenAIProvider::parseStreamDeltas(const QByteArray &chunk) const
{
    QList<LlmStreamDelta> deltas;
    const QJsonObject event = parseSseEvent(chunk);
    if (event.isEmpty()) {
        return deltas;
    }

    const QString type = event.value(QStringLiteral("type")).toString();
    if (type == QStringLiteral("response.output_text.delta")) {
        LlmStreamDelta delta;
        delta.channel = QStringLiteral("content");
        delta.text = event.value(QStringLiteral("delta")).toString();
        if (!delta.text.isEmpty()) {
            deltas.append(delta);
        }
        return deltas;
    }

    if (type.contains(QStringLiteral("reasoning"), Qt::CaseInsensitive)) {
        const QString text = event.value(QStringLiteral("delta")).toString(event.value(QStringLiteral("text")).toString());
        if (!text.isEmpty()) {
            LlmStreamDelta delta;
            delta.channel = QStringLiteral("reasoning");
            delta.text = text;
            deltas.append(delta);
        }
    }

    return deltas;
}

bool OpenAIProvider::supportsStructuredToolCalls() const
{
    return true;
}

} // namespace qtllm





