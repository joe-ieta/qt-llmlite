#include "defaultmcpclient.h"

#include "../runtime/toolruntime_types.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTimer>
#include <QUrl>

namespace qtllm::tools::mcp {

namespace {

QJsonObject normalizeResultObject(const QJsonValue &value)
{
    if (value.isObject()) {
        return value.toObject();
    }

    QJsonObject wrapped;
    wrapped.insert(QStringLiteral("content"), value);
    return wrapped;
}

QJsonObject buildJsonRpcRequest(int id, const QString &method, const QJsonObject &params)
{
    QJsonObject request;
    request.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    request.insert(QStringLiteral("id"), id);
    request.insert(QStringLiteral("method"), method);
    request.insert(QStringLiteral("params"), params);
    return request;
}

} // namespace

DefaultMcpClient::DefaultMcpClient() = default;

QVector<McpToolDescriptor> DefaultMcpClient::listTools(const McpServerDefinition &server,
                                                       QString *errorMessage)
{
    QVector<McpToolDescriptor> tools;

    const QJsonObject response = performJsonRpc(server,
                                                QStringLiteral("tools/list"),
                                                QJsonObject(),
                                                errorMessage);
    if (response.isEmpty()) {
        return tools;
    }

    const QJsonObject result = response.value(QStringLiteral("result")).toObject();
    const QJsonArray entries = result.value(QStringLiteral("tools")).toArray();
    for (const QJsonValue &v : entries) {
        const QJsonObject item = v.toObject();

        McpToolDescriptor descriptor;
        descriptor.name = item.value(QStringLiteral("name")).toString();
        descriptor.description = item.value(QStringLiteral("description")).toString();

        QJsonObject schema = item.value(QStringLiteral("inputSchema")).toObject();
        if (schema.isEmpty()) {
            schema = item.value(QStringLiteral("input_schema")).toObject();
        }
        descriptor.inputSchema = schema;

        if (!descriptor.name.trimmed().isEmpty()) {
            tools.append(descriptor);
        }
    }

    return tools;
}

QVector<McpResourceDescriptor> DefaultMcpClient::listResources(const McpServerDefinition &server,
                                                               QString *errorMessage)
{
    QVector<McpResourceDescriptor> resources;

    const QJsonObject response = performJsonRpc(server,
                                                QStringLiteral("resources/list"),
                                                QJsonObject(),
                                                errorMessage);
    if (response.isEmpty()) {
        return resources;
    }

    const QJsonObject result = response.value(QStringLiteral("result")).toObject();
    const QJsonArray entries = result.value(QStringLiteral("resources")).toArray();
    for (const QJsonValue &v : entries) {
        const QJsonObject item = v.toObject();

        McpResourceDescriptor descriptor;
        descriptor.uri = item.value(QStringLiteral("uri")).toString();
        descriptor.name = item.value(QStringLiteral("name")).toString();
        descriptor.description = item.value(QStringLiteral("description")).toString();

        if (!descriptor.uri.trimmed().isEmpty() || !descriptor.name.trimmed().isEmpty()) {
            resources.append(descriptor);
        }
    }

    return resources;
}

QVector<McpPromptDescriptor> DefaultMcpClient::listPrompts(const McpServerDefinition &server,
                                                           QString *errorMessage)
{
    QVector<McpPromptDescriptor> prompts;

    const QJsonObject response = performJsonRpc(server,
                                                QStringLiteral("prompts/list"),
                                                QJsonObject(),
                                                errorMessage);
    if (response.isEmpty()) {
        return prompts;
    }

    const QJsonObject result = response.value(QStringLiteral("result")).toObject();
    const QJsonArray entries = result.value(QStringLiteral("prompts")).toArray();
    for (const QJsonValue &v : entries) {
        const QJsonObject item = v.toObject();

        McpPromptDescriptor descriptor;
        descriptor.name = item.value(QStringLiteral("name")).toString();
        descriptor.description = item.value(QStringLiteral("description")).toString();
        descriptor.arguments = item.value(QStringLiteral("arguments")).toArray();

        if (!descriptor.name.trimmed().isEmpty()) {
            prompts.append(descriptor);
        }
    }

    return prompts;
}

McpToolCallResult DefaultMcpClient::callTool(const McpServerDefinition &server,
                                             const QString &toolName,
                                             const QJsonObject &arguments,
                                             const runtime::ToolExecutionContext &context,
                                             QString *errorMessage)
{
    Q_UNUSED(context)

    McpToolCallResult out;

    QJsonObject params;
    params.insert(QStringLiteral("name"), toolName);
    params.insert(QStringLiteral("arguments"), arguments);

    const QJsonObject response = performJsonRpc(server,
                                                QStringLiteral("tools/call"),
                                                params,
                                                errorMessage);
    if (response.isEmpty()) {
        out.errorCode = QStringLiteral("mcp_call_failed");
        out.errorMessage = errorMessage ? *errorMessage : QStringLiteral("Unknown MCP error");
        out.retryable = true;
        return out;
    }

    const QJsonObject error = response.value(QStringLiteral("error")).toObject();
    if (!error.isEmpty()) {
        out.errorCode = QStringLiteral("mcp_rpc_error");
        out.errorMessage = error.value(QStringLiteral("message")).toString(QStringLiteral("MCP RPC error"));
        out.retryable = true;
        return out;
    }

    const QJsonObject result = response.value(QStringLiteral("result")).toObject();
    if (result.isEmpty()) {
        out.errorCode = QStringLiteral("mcp_empty_result");
        out.errorMessage = QStringLiteral("MCP returned empty result");
        out.retryable = true;
        return out;
    }

    out.success = !result.value(QStringLiteral("isError")).toBool(false);
    if (out.success) {
        out.output = normalizeResultObject(result.value(QStringLiteral("content")));
        if (out.output.isEmpty()) {
            out.output = result;
        }
    } else {
        out.errorCode = QStringLiteral("mcp_tool_error");
        out.errorMessage = QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
        out.retryable = false;
    }

    return out;
}

QJsonObject DefaultMcpClient::performJsonRpc(const McpServerDefinition &server,
                                             const QString &method,
                                             const QJsonObject &params,
                                             QString *errorMessage) const
{
    const QJsonObject request = buildJsonRpcRequest(m_requestIdSeed++, method, params);
    const QString transport = server.transport.trimmed().toLower();

    if (transport == QStringLiteral("stdio")) {
        return performStdioJsonRpc(server, request, errorMessage);
    }

    if (transport == QStringLiteral("streamable-http") || transport == QStringLiteral("sse")) {
        return performHttpLikeJsonRpc(server, request, errorMessage);
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Unsupported MCP transport: ") + server.transport;
    }
    return QJsonObject();
}

QJsonObject DefaultMcpClient::performStdioJsonRpc(const McpServerDefinition &server,
                                                  const QJsonObject &request,
                                                  QString *errorMessage) const
{
    QProcess process;
    process.setProgram(server.command);
    process.setArguments(server.args);

    const QJsonObject envObj = server.env;
    if (!envObj.isEmpty()) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        for (auto it = envObj.constBegin(); it != envObj.constEnd(); ++it) {
            env.insert(it.key(), it.value().toString());
        }
        process.setProcessEnvironment(env);
    }

    process.start();
    if (!process.waitForStarted(server.timeoutMs)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to start MCP stdio process: ") + process.errorString();
        }
        return QJsonObject();
    }

    QByteArray payload = QJsonDocument(request).toJson(QJsonDocument::Compact);
    payload.append('\n');
    process.write(payload);
    process.closeWriteChannel();

    if (!process.waitForFinished(server.timeoutMs)) {
        process.kill();
        process.waitForFinished(1000);
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP stdio process timed out");
        }
        return QJsonObject();
    }

    const QByteArray output = process.readAllStandardOutput().trimmed();
    if (output.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP stdio process returned empty output");
        }
        return QJsonObject();
    }

    const QList<QByteArray> lines = output.split('\n');
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(lines.last().trimmed(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to parse MCP stdio response: ") + parseError.errorString();
        }
        return QJsonObject();
    }

    return doc.object();
}

QJsonObject DefaultMcpClient::performHttpLikeJsonRpc(const McpServerDefinition &server,
                                                     const QJsonObject &request,
                                                     QString *errorMessage) const
{
    const QUrl url(server.url);
    if (!url.isValid() || url.scheme().isEmpty() || url.host().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid MCP server URL: ") + server.url;
        }
        return QJsonObject();
    }

    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    for (auto it = server.headers.constBegin(); it != server.headers.constEnd(); ++it) {
        networkRequest.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
    }

    QNetworkReply *reply = m_network.post(networkRequest,
                                          QJsonDocument(request).toJson(QJsonDocument::Compact));

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    QObject::connect(&timeout, &QTimer::timeout, &loop, [&]() {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
        loop.quit();
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timeout.start(server.timeoutMs);
    loop.exec();

    const bool timeoutHit = !timeout.isActive();
    timeout.stop();

    if (timeoutHit) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP HTTP request timed out");
        }
        reply->deleteLater();
        return QJsonObject();
    }

    if (reply->error() != QNetworkReply::NoError) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP HTTP request failed: ") + reply->errorString();
        }
        reply->deleteLater();
        return QJsonObject();
    }

    const QByteArray payload = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to parse MCP HTTP response: ") + parseError.errorString();
        }
        return QJsonObject();
    }

    return doc.object();
}

} // namespace qtllm::tools::mcp
