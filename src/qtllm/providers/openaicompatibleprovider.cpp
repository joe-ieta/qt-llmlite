#include "openaicompatibleprovider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrl>
#include <QUrlQuery>

namespace qtllm {

namespace {

enum class VendorSchema
{
    OpenAI,
    Anthropic,
    Google
};

bool containsAny(const QString &text, const QStringList &terms)
{
    const QString lowered = text.trimmed().toLower();
    for (const QString &term : terms) {
        if (lowered.contains(term)) {
            return true;
        }
    }
    return false;
}

QString inferVendor(const QString &explicitVendor, const QString &modelName)
{
    const QString vendor = explicitVendor.trimmed().toLower();
    if (!vendor.isEmpty()) {
        return vendor;
    }

    const QString model = modelName.trimmed().toLower();
    if (model.isEmpty()) {
        return QStringLiteral("openai");
    }

    if (containsAny(model, QStringList({QStringLiteral("claude"), QStringLiteral("anthropic")}))) {
        return QStringLiteral("anthropic");
    }

    if (containsAny(model, QStringList({QStringLiteral("gemini"), QStringLiteral("palm"), QStringLiteral("google")}))) {
        return QStringLiteral("google");
    }

    if (containsAny(model, QStringList({QStringLiteral("deepseek")}))) {
        return QStringLiteral("deepseek");
    }

    if (containsAny(model, QStringList({QStringLiteral("qwen"), QStringLiteral("tongyi")}))) {
        return QStringLiteral("qwen");
    }

    if (containsAny(model, QStringList({QStringLiteral("glm"), QStringLiteral("chatglm"), QStringLiteral("zhipu")}))) {
        return QStringLiteral("glm");
    }

    return QStringLiteral("openai");
}

VendorSchema vendorSchema(const QString &vendor)
{
    if (vendor == QStringLiteral("anthropic")) {
        return VendorSchema::Anthropic;
    }

    if (vendor == QStringLiteral("google") || vendor == QStringLiteral("gemini")) {
        return VendorSchema::Google;
    }

    return VendorSchema::OpenAI;
}

QString resolveVendor(const LlmConfig &config, const LlmRequest &request)
{
    const QString model = request.model.trimmed().isEmpty() ? config.model : request.model;
    return inferVendor(config.modelVendor, model);
}

QJsonObject toOpenAiMessage(const LlmMessage &message)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("role"), message.role);

    const QString role = message.role.trimmed().toLower();
    const bool assistantToolCallOnly = role == QStringLiteral("assistant")
        && !message.toolCalls.isEmpty()
        && message.content.isEmpty();
    if (!assistantToolCallOnly) {
        obj.insert(QStringLiteral("content"), message.content);
    }

    if (!message.name.trimmed().isEmpty()) {
        obj.insert(QStringLiteral("name"), message.name);
    }
    if (!message.toolCallId.trimmed().isEmpty()) {
        obj.insert(QStringLiteral("tool_call_id"), message.toolCallId);
    }

    if (!message.toolCalls.isEmpty()) {
        QJsonArray toolCalls;
        for (const LlmToolCall &call : message.toolCalls) {
            QJsonObject function;
            function.insert(QStringLiteral("name"), call.name);
            function.insert(QStringLiteral("arguments"),
                            QString::fromUtf8(QJsonDocument(call.arguments).toJson(QJsonDocument::Compact)));

            QJsonObject callObj;
            callObj.insert(QStringLiteral("id"), call.id);
            callObj.insert(QStringLiteral("type"), call.type);
            callObj.insert(QStringLiteral("function"), function);
            toolCalls.append(callObj);
        }
        obj.insert(QStringLiteral("tool_calls"), toolCalls);
    }

    return obj;
}

QJsonArray toOpenAiMessages(const QVector<LlmMessage> &messages)
{
    QJsonArray array;
    for (const LlmMessage &message : messages) {
        array.append(toOpenAiMessage(message));
    }
    return array;
}

QJsonArray toAnthropicTools(const QJsonArray &openAiTools)
{
    QJsonArray array;
    for (const QJsonValue &toolValue : openAiTools) {
        const QJsonObject toolObj = toolValue.toObject();
        const QJsonObject functionObj = toolObj.value(QStringLiteral("function")).toObject();
        const QString toolName = functionObj.value(QStringLiteral("name")).toString();
        if (toolName.trimmed().isEmpty()) {
            continue;
        }

        QJsonObject mapped;
        mapped.insert(QStringLiteral("name"), toolName);
        const QString desc = functionObj.value(QStringLiteral("description")).toString();
        if (!desc.trimmed().isEmpty()) {
            mapped.insert(QStringLiteral("description"), desc);
        }
        mapped.insert(QStringLiteral("input_schema"), functionObj.value(QStringLiteral("parameters")).toObject());
        array.append(mapped);
    }
    return array;
}

QJsonArray toAnthropicMessages(const QVector<LlmMessage> &messages, QString *systemPrompt)
{
    QJsonArray array;
    QStringList systemBlocks;
    int generatedToolId = 0;

    for (const LlmMessage &message : messages) {
        const QString role = message.role.trimmed().toLower();

        if (role == QStringLiteral("system")) {
            if (!message.content.trimmed().isEmpty()) {
                systemBlocks.append(message.content);
            }
            continue;
        }

        if (role == QStringLiteral("tool")) {
            QJsonArray content;
            QJsonObject toolResult;
            toolResult.insert(QStringLiteral("type"), QStringLiteral("tool_result"));
            if (!message.toolCallId.trimmed().isEmpty()) {
                toolResult.insert(QStringLiteral("tool_use_id"), message.toolCallId);
            }

            const QJsonDocument parsed = QJsonDocument::fromJson(message.content.toUtf8());
            if (parsed.isObject()) {
                toolResult.insert(QStringLiteral("content"), parsed.object());
            } else {
                toolResult.insert(QStringLiteral("content"), message.content);
            }
            content.append(toolResult);

            QJsonObject obj;
            obj.insert(QStringLiteral("role"), QStringLiteral("user"));
            obj.insert(QStringLiteral("content"), content);
            array.append(obj);
            continue;
        }

        const QString mappedRole = role == QStringLiteral("assistant") ? QStringLiteral("assistant") : QStringLiteral("user");
        QJsonArray content;

        if (!message.content.trimmed().isEmpty()) {
            QJsonObject textBlock;
            textBlock.insert(QStringLiteral("type"), QStringLiteral("text"));
            textBlock.insert(QStringLiteral("text"), message.content);
            content.append(textBlock);
        }

        if (mappedRole == QStringLiteral("assistant")) {
            for (const LlmToolCall &call : message.toolCalls) {
                if (call.name.trimmed().isEmpty()) {
                    continue;
                }

                QJsonObject toolUse;
                toolUse.insert(QStringLiteral("type"), QStringLiteral("tool_use"));
                const QString callId = call.id.trimmed().isEmpty()
                    ? QStringLiteral("call_%1").arg(++generatedToolId)
                    : call.id;
                toolUse.insert(QStringLiteral("id"), callId);
                toolUse.insert(QStringLiteral("name"), call.name);
                toolUse.insert(QStringLiteral("input"), call.arguments);
                content.append(toolUse);
            }
        }

        if (content.isEmpty()) {
            continue;
        }

        QJsonObject obj;
        obj.insert(QStringLiteral("role"), mappedRole);
        obj.insert(QStringLiteral("content"), content);
        array.append(obj);
    }

    if (systemPrompt) {
        *systemPrompt = systemBlocks.join(QStringLiteral("\n\n"));
    }

    return array;
}

QJsonArray toGoogleTools(const QJsonArray &openAiTools)
{
    QJsonArray declarations;
    for (const QJsonValue &toolValue : openAiTools) {
        const QJsonObject toolObj = toolValue.toObject();
        const QJsonObject functionObj = toolObj.value(QStringLiteral("function")).toObject();
        const QString toolName = functionObj.value(QStringLiteral("name")).toString();
        if (toolName.trimmed().isEmpty()) {
            continue;
        }

        QJsonObject decl;
        decl.insert(QStringLiteral("name"), toolName);
        const QString desc = functionObj.value(QStringLiteral("description")).toString();
        if (!desc.trimmed().isEmpty()) {
            decl.insert(QStringLiteral("description"), desc);
        }
        decl.insert(QStringLiteral("parameters"), functionObj.value(QStringLiteral("parameters")).toObject());
        declarations.append(decl);
    }

    if (declarations.isEmpty()) {
        return QJsonArray();
    }

    QJsonObject wrapper;
    wrapper.insert(QStringLiteral("functionDeclarations"), declarations);

    QJsonArray tools;
    tools.append(wrapper);
    return tools;
}

QJsonArray toGoogleContents(const QVector<LlmMessage> &messages, QString *systemPrompt)
{
    QJsonArray contents;
    QStringList systemBlocks;

    for (const LlmMessage &message : messages) {
        const QString role = message.role.trimmed().toLower();

        if (role == QStringLiteral("system")) {
            if (!message.content.trimmed().isEmpty()) {
                systemBlocks.append(message.content);
            }
            continue;
        }

        QString mappedRole = QStringLiteral("user");
        if (role == QStringLiteral("assistant")) {
            mappedRole = QStringLiteral("model");
        } else if (role == QStringLiteral("tool")) {
            mappedRole = QStringLiteral("function");
        }

        QJsonArray parts;

        if (role == QStringLiteral("tool")) {
            QJsonObject part;
            QJsonObject functionResponse;
            functionResponse.insert(QStringLiteral("name"), message.name.trimmed().isEmpty() ? QStringLiteral("tool") : message.name);
            const QJsonDocument parsed = QJsonDocument::fromJson(message.content.toUtf8());
            if (parsed.isObject()) {
                functionResponse.insert(QStringLiteral("response"), parsed.object());
            } else {
                QJsonObject fallback;
                fallback.insert(QStringLiteral("result"), message.content);
                functionResponse.insert(QStringLiteral("response"), fallback);
            }
            part.insert(QStringLiteral("functionResponse"), functionResponse);
            parts.append(part);
        } else {
            if (!message.content.trimmed().isEmpty()) {
                QJsonObject textPart;
                textPart.insert(QStringLiteral("text"), message.content);
                parts.append(textPart);
            }

            if (role == QStringLiteral("assistant")) {
                for (const LlmToolCall &call : message.toolCalls) {
                    if (call.name.trimmed().isEmpty()) {
                        continue;
                    }

                    QJsonObject part;
                    QJsonObject functionCall;
                    functionCall.insert(QStringLiteral("name"), call.name);
                    functionCall.insert(QStringLiteral("args"), call.arguments);
                    part.insert(QStringLiteral("functionCall"), functionCall);
                    parts.append(part);
                }
            }
        }

        if (parts.isEmpty()) {
            continue;
        }

        QJsonObject item;
        item.insert(QStringLiteral("role"), mappedRole);
        item.insert(QStringLiteral("parts"), parts);
        contents.append(item);
    }

    if (systemPrompt) {
        *systemPrompt = systemBlocks.join(QStringLiteral("\n\n"));
    }

    return contents;
}

LlmResponse parseOpenAiResponse(const QJsonObject &root)
{
    LlmResponse response;

    const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
    if (choices.isEmpty()) {
        response.errorMessage = QStringLiteral("No choices found in response");
        return response;
    }

    const QJsonObject firstChoice = choices.first().toObject();
    response.finishReason = firstChoice.value(QStringLiteral("finish_reason")).toString();

    const QJsonObject message = firstChoice.value(QStringLiteral("message")).toObject();

    LlmMessage assistant;
    assistant.role = QStringLiteral("assistant");
    assistant.content = message.value(QStringLiteral("content")).toString();

    const QJsonArray toolCalls = message.value(QStringLiteral("tool_calls")).toArray();
    for (const QJsonValue &value : toolCalls) {
        const QJsonObject callObj = value.toObject();
        const QJsonObject function = callObj.value(QStringLiteral("function")).toObject();

        LlmToolCall call;
        call.id = callObj.value(QStringLiteral("id")).toString();
        call.type = callObj.value(QStringLiteral("type")).toString(QStringLiteral("function"));
        call.name = function.value(QStringLiteral("name")).toString();

        const QJsonValue argumentsValue = function.value(QStringLiteral("arguments"));
        if (argumentsValue.isString()) {
            const QJsonDocument argsDoc = QJsonDocument::fromJson(argumentsValue.toString().toUtf8());
            if (argsDoc.isObject()) {
                call.arguments = argsDoc.object();
            }
        } else if (argumentsValue.isObject()) {
            call.arguments = argumentsValue.toObject();
        }

        if (!call.name.trimmed().isEmpty()) {
            assistant.toolCalls.append(call);
        }
    }

    response.assistantMessage = assistant;
    response.text = assistant.content;

    if (!assistant.content.isEmpty() || !assistant.toolCalls.isEmpty()) {
        response.success = true;
    } else {
        response.errorMessage = QStringLiteral("Empty assistant message in response");
    }

    return response;
}

LlmResponse parseAnthropicResponse(const QJsonObject &root)
{
    LlmResponse response;
    response.finishReason = root.value(QStringLiteral("stop_reason")).toString();

    LlmMessage assistant;
    assistant.role = QStringLiteral("assistant");

    QStringList texts;
    const QJsonArray content = root.value(QStringLiteral("content")).toArray();
    for (const QJsonValue &value : content) {
        const QJsonObject block = value.toObject();
        const QString type = block.value(QStringLiteral("type")).toString();

        if (type == QStringLiteral("text")) {
            const QString text = block.value(QStringLiteral("text")).toString();
            if (!text.isEmpty()) {
                texts.append(text);
            }
            continue;
        }

        if (type == QStringLiteral("tool_use")) {
            LlmToolCall call;
            call.id = block.value(QStringLiteral("id")).toString();
            call.type = QStringLiteral("function");
            call.name = block.value(QStringLiteral("name")).toString();
            call.arguments = block.value(QStringLiteral("input")).toObject();
            if (!call.name.trimmed().isEmpty()) {
                assistant.toolCalls.append(call);
            }
        }
    }

    assistant.content = texts.join(QStringLiteral("\n"));
    response.assistantMessage = assistant;
    response.text = assistant.content;

    if (!assistant.content.isEmpty() || !assistant.toolCalls.isEmpty()) {
        response.success = true;
    } else {
        response.errorMessage = QStringLiteral("Empty assistant message in anthropic response");
    }

    return response;
}

LlmResponse parseGoogleResponse(const QJsonObject &root)
{
    LlmResponse response;

    const QJsonArray candidates = root.value(QStringLiteral("candidates")).toArray();
    if (candidates.isEmpty()) {
        response.errorMessage = QStringLiteral("No candidates found in google response");
        return response;
    }

    const QJsonObject candidate = candidates.first().toObject();
    response.finishReason = candidate.value(QStringLiteral("finishReason")).toString();

    const QJsonObject contentObj = candidate.value(QStringLiteral("content")).toObject();
    const QJsonArray parts = contentObj.value(QStringLiteral("parts")).toArray();

    LlmMessage assistant;
    assistant.role = QStringLiteral("assistant");

    QStringList texts;
    for (const QJsonValue &value : parts) {
        const QJsonObject part = value.toObject();

        const QString text = part.value(QStringLiteral("text")).toString();
        if (!text.isEmpty()) {
            texts.append(text);
        }

        const QJsonObject functionCall = part.value(QStringLiteral("functionCall")).toObject();
        const QString callName = functionCall.value(QStringLiteral("name")).toString();
        if (!callName.trimmed().isEmpty()) {
            LlmToolCall call;
            call.type = QStringLiteral("function");
            call.name = callName;
            call.arguments = functionCall.value(QStringLiteral("args")).toObject();
            assistant.toolCalls.append(call);
        }
    }

    assistant.content = texts.join(QStringLiteral("\n"));
    response.assistantMessage = assistant;
    response.text = assistant.content;

    if (!assistant.content.isEmpty() || !assistant.toolCalls.isEmpty()) {
        response.success = true;
    } else {
        response.errorMessage = QStringLiteral("Empty assistant message in google response");
    }

    return response;
}

} // namespace

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
    const QString vendor = resolveVendor(m_config, request);
    const VendorSchema schema = vendorSchema(vendor);

    QUrl url(m_config.baseUrl);

    if (schema == VendorSchema::Anthropic) {
        if (!url.path().endsWith(QStringLiteral("/messages"))) {
            QString path = url.path();
            if (!path.endsWith('/')) {
                path += '/';
            }
            path += QStringLiteral("messages");
            url.setPath(path);
        }
    } else if (schema == VendorSchema::Google) {
        const QString model = request.model.trimmed().isEmpty() ? m_config.model : request.model;
        QString path = url.path();

        if (!path.contains(QStringLiteral(":generateContent"))) {
            if (path.contains(QStringLiteral("/models/"))) {
                if (!path.endsWith(QStringLiteral(":generateContent"))) {
                    path += QStringLiteral(":generateContent");
                }
            } else {
                if (!path.endsWith('/')) {
                    path += '/';
                }
                path += QStringLiteral("v1beta/models/") + model + QStringLiteral(":generateContent");
            }
            url.setPath(path);
        }

        if (!m_config.apiKey.isEmpty()) {
            QUrlQuery query(url);
            if (!query.hasQueryItem(QStringLiteral("key"))) {
                query.addQueryItem(QStringLiteral("key"), m_config.apiKey);
                url.setQuery(query);
            }
        }
    } else {
        if (!url.path().endsWith(QStringLiteral("/chat/completions"))) {
            QString path = url.path();
            if (!path.endsWith('/')) {
                path += '/';
            }
            path += QStringLiteral("chat/completions");
            url.setPath(path);
        }
    }

    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    if (schema == VendorSchema::Anthropic) {
        if (!m_config.apiKey.isEmpty()) {
            networkRequest.setRawHeader("x-api-key", m_config.apiKey.toUtf8());
        }
        networkRequest.setRawHeader("anthropic-version", QByteArray("2023-06-01"));
    } else if (schema == VendorSchema::Google) {
        if (!m_config.apiKey.isEmpty()) {
            networkRequest.setRawHeader("x-goog-api-key", m_config.apiKey.toUtf8());
        }
    } else {
        if (!m_config.apiKey.isEmpty()) {
            const QByteArray bearer = "Bearer " + m_config.apiKey.toUtf8();
            networkRequest.setRawHeader("Authorization", bearer);
        }
    }

    return networkRequest;
}

QByteArray OpenAICompatibleProvider::buildPayload(const LlmRequest &request) const
{
    const QString vendor = resolveVendor(m_config, request);
    const VendorSchema schema = vendorSchema(vendor);
    const QString model = request.model.trimmed().isEmpty() ? m_config.model : request.model;

    QJsonObject root;

    if (schema == VendorSchema::Anthropic) {
        root.insert(QStringLiteral("model"), model);
        root.insert(QStringLiteral("stream"), request.stream);

        QString systemPrompt;
        root.insert(QStringLiteral("messages"), toAnthropicMessages(request.messages, &systemPrompt));
        if (!systemPrompt.trimmed().isEmpty()) {
            root.insert(QStringLiteral("system"), systemPrompt);
        }

        const QJsonArray tools = toAnthropicTools(request.tools);
        if (!tools.isEmpty()) {
            root.insert(QStringLiteral("tools"), tools);
        }

        return QJsonDocument(root).toJson(QJsonDocument::Compact);
    }

    if (schema == VendorSchema::Google) {
        QString systemPrompt;
        root.insert(QStringLiteral("contents"), toGoogleContents(request.messages, &systemPrompt));

        if (!systemPrompt.trimmed().isEmpty()) {
            QJsonObject systemInstruction;
            QJsonArray parts;
            QJsonObject textPart;
            textPart.insert(QStringLiteral("text"), systemPrompt);
            parts.append(textPart);
            systemInstruction.insert(QStringLiteral("parts"), parts);
            root.insert(QStringLiteral("systemInstruction"), systemInstruction);
        }

        const QJsonArray tools = toGoogleTools(request.tools);
        if (!tools.isEmpty()) {
            root.insert(QStringLiteral("tools"), tools);
        }

        return QJsonDocument(root).toJson(QJsonDocument::Compact);
    }

    root.insert(QStringLiteral("model"), model);
    root.insert(QStringLiteral("stream"), request.stream);
    root.insert(QStringLiteral("messages"), toOpenAiMessages(request.messages));
    if (!request.tools.isEmpty()) {
        root.insert(QStringLiteral("tools"), request.tools);
    }

    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

LlmResponse OpenAICompatibleProvider::parseResponse(const QByteArray &data) const
{
    const QByteArray trimmedData = data.trimmed();
    const QString vendor = inferVendor(m_config.modelVendor, m_config.model);
    const VendorSchema schema = vendorSchema(vendor);

    if (trimmedData.startsWith("data:")) {
        QJsonObject lastRoot;
        QStringList contentParts;
        QStringList reasoningParts;
        QString finishReason;

        const QList<QByteArray> lines = trimmedData.split('\n');
        for (const QByteArray &line : lines) {
            const QByteArray trimmed = line.trimmed();
            if (!trimmed.startsWith("data:")) {
                continue;
            }

            const QByteArray payload = trimmed.mid(5).trimmed();
            if (payload.isEmpty() || payload == "[DONE]") {
                continue;
            }

            const QJsonDocument doc = QJsonDocument::fromJson(payload);
            if (!doc.isObject()) {
                continue;
            }

            lastRoot = doc.object();
            if (schema != VendorSchema::OpenAI) {
                continue;
            }

            const QJsonArray choices = lastRoot.value(QStringLiteral("choices")).toArray();
            if (choices.isEmpty()) {
                continue;
            }

            const QJsonObject choice = choices.first().toObject();
            finishReason = choice.value(QStringLiteral("finish_reason")).toString(finishReason);
            const QJsonObject delta = choice.value(QStringLiteral("delta")).toObject();

            const QString content = delta.value(QStringLiteral("content")).toString();
            if (!content.isEmpty()) {
                contentParts.append(content);
            }

            const QString reasoning = delta.value(QStringLiteral("reasoning")).toString();
            if (!reasoning.isEmpty()) {
                reasoningParts.append(reasoning);
            }
        }

        if (lastRoot.isEmpty()) {
            LlmResponse response;
            response.errorMessage = QStringLiteral("Invalid SSE response: no JSON events found");
            return response;
        }

        if (schema == VendorSchema::Anthropic) {
            return parseAnthropicResponse(lastRoot);
        }

        if (schema == VendorSchema::Google) {
            return parseGoogleResponse(lastRoot);
        }

        LlmResponse response;
        response.finishReason = finishReason;
        response.assistantMessage.role = QStringLiteral("assistant");
        response.assistantMessage.content = contentParts.join(QString());
        if (response.assistantMessage.content.isEmpty()) {
            response.assistantMessage.content = reasoningParts.join(QString());
        }
        response.text = response.assistantMessage.content;
        response.success = !response.text.isEmpty();

        if (!response.success) {
            response = parseOpenAiResponse(lastRoot);
            if (!response.success) {
                response.errorMessage = QStringLiteral("Empty assistant message in SSE response");
            }
        }

        return response;
    }

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(trimmedData, &error);
    if (!doc.isObject()) {
        LlmResponse response;
        response.errorMessage = QStringLiteral("Invalid JSON response: ")
            + error.errorString()
            + QStringLiteral("\n")
            + QString::fromUtf8(trimmedData.left(2048));
        return response;
    }

    const QJsonObject root = doc.object();
    if (schema == VendorSchema::Anthropic || root.contains(QStringLiteral("stop_reason"))) {
        return parseAnthropicResponse(root);
    }

    if (schema == VendorSchema::Google || root.contains(QStringLiteral("candidates"))) {
        return parseGoogleResponse(root);
    }

    return parseOpenAiResponse(root);
}

QList<LlmStreamDelta> OpenAICompatibleProvider::parseStreamDeltas(const QByteArray &chunk) const
{
    QList<LlmStreamDelta> deltas;

    const QString vendor = inferVendor(m_config.modelVendor, m_config.model);
    const VendorSchema schema = vendorSchema(vendor);

    const QList<QByteArray> lines = chunk.split('\n');
    for (const QByteArray &line : lines) {
        const QByteArray trimmed = line.trimmed();
        if (!trimmed.startsWith("data:")) {
            continue;
        }

        const QByteArray payload = trimmed.mid(5).trimmed();
        if (payload == "[DONE]" || payload.isEmpty()) {
            continue;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(payload);
        if (!doc.isObject()) {
            continue;
        }

        const QJsonObject root = doc.object();

        if (schema == VendorSchema::Anthropic || root.contains(QStringLiteral("delta"))) {
            const QJsonObject delta = root.value(QStringLiteral("delta")).toObject();
            const QString text = delta.value(QStringLiteral("text")).toString();
            if (!text.isEmpty()) {
                LlmStreamDelta streamDelta;
                streamDelta.channel = QStringLiteral("content");
                streamDelta.text = text;
                deltas.append(streamDelta);
            }
            continue;
        }

        if (schema == VendorSchema::Google || root.contains(QStringLiteral("candidates"))) {
            const QJsonArray candidates = root.value(QStringLiteral("candidates")).toArray();
            if (!candidates.isEmpty()) {
                const QJsonArray parts = candidates.first().toObject()
                                             .value(QStringLiteral("content")).toObject()
                                             .value(QStringLiteral("parts")).toArray();
                for (const QJsonValue &partValue : parts) {
                    const QString text = partValue.toObject().value(QStringLiteral("text")).toString();
                    if (!text.isEmpty()) {
                        LlmStreamDelta streamDelta;
                        streamDelta.channel = QStringLiteral("content");
                        streamDelta.text = text;
                        deltas.append(streamDelta);
                    }
                }
            }
            continue;
        }

        const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
        if (choices.isEmpty()) {
            continue;
        }

        const QJsonObject delta = choices.first().toObject().value(QStringLiteral("delta")).toObject();
        const QString content = delta.value(QStringLiteral("content")).toString();
        if (!content.isEmpty()) {
            LlmStreamDelta streamDelta;
            streamDelta.channel = QStringLiteral("content");
            streamDelta.text = content;
            deltas.append(streamDelta);
        }

        const QString reasoning = delta.value(QStringLiteral("reasoning")).toString();
        if (!reasoning.isEmpty()) {
            LlmStreamDelta streamDelta;
            streamDelta.channel = QStringLiteral("reasoning");
            streamDelta.text = reasoning;
            deltas.append(streamDelta);
        }
    }

    return deltas;
}
QList<QString> OpenAICompatibleProvider::parseStreamTokens(const QByteArray &chunk) const
{
    QList<QString> tokens;

    const QString vendor = inferVendor(m_config.modelVendor, m_config.model);
    const VendorSchema schema = vendorSchema(vendor);

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

        const QJsonObject root = doc.object();

        if (schema == VendorSchema::Anthropic || root.contains(QStringLiteral("delta"))) {
            const QJsonObject delta = root.value(QStringLiteral("delta")).toObject();
            const QString text = delta.value(QStringLiteral("text")).toString();
            if (!text.isEmpty()) {
                tokens.append(text);
            }
            continue;
        }

        if (schema == VendorSchema::Google || root.contains(QStringLiteral("candidates"))) {
            const QJsonArray candidates = root.value(QStringLiteral("candidates")).toArray();
            if (!candidates.isEmpty()) {
                const QJsonArray parts = candidates.first().toObject()
                                             .value(QStringLiteral("content")).toObject()
                                             .value(QStringLiteral("parts")).toArray();
                for (const QJsonValue &partValue : parts) {
                    const QString text = partValue.toObject().value(QStringLiteral("text")).toString();
                    if (!text.isEmpty()) {
                        tokens.append(text);
                    }
                }
            }
            continue;
        }

        const QJsonArray choices = root.value(QStringLiteral("choices")).toArray();
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

bool OpenAICompatibleProvider::supportsStructuredToolCalls() const
{
    return true;
}

} // namespace qtllm



