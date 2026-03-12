#pragma once

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVector>

namespace qtllm::tools::mcp {

struct McpServerDefinition
{
    QString serverId;
    QString name;
    QString transport = QStringLiteral("stdio"); // stdio | streamable-http | sse
    bool enabled = true;

    QString command;
    QStringList args;
    QJsonObject env;

    QString url;
    QJsonObject headers;

    int timeoutMs = 30000;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("serverId"), serverId);
        root.insert(QStringLiteral("name"), name);
        root.insert(QStringLiteral("transport"), transport);
        root.insert(QStringLiteral("enabled"), enabled);
        root.insert(QStringLiteral("command"), command);

        QJsonArray argsArray;
        for (const QString &arg : args) {
            argsArray.append(arg);
        }
        root.insert(QStringLiteral("args"), argsArray);
        root.insert(QStringLiteral("env"), env);
        root.insert(QStringLiteral("url"), url);
        root.insert(QStringLiteral("headers"), headers);
        root.insert(QStringLiteral("timeoutMs"), timeoutMs);
        return root;
    }

    static McpServerDefinition fromJson(const QJsonObject &root)
    {
        McpServerDefinition server;
        server.serverId = root.value(QStringLiteral("serverId")).toString();
        server.name = root.value(QStringLiteral("name")).toString();
        server.transport = root.value(QStringLiteral("transport")).toString(server.transport);
        server.enabled = root.value(QStringLiteral("enabled")).toBool(true);
        server.command = root.value(QStringLiteral("command")).toString();

        const QJsonArray argsArray = root.value(QStringLiteral("args")).toArray();
        for (const QJsonValue &v : argsArray) {
            const QString arg = v.toString();
            if (!arg.isEmpty()) {
                server.args.append(arg);
            }
        }

        server.env = root.value(QStringLiteral("env")).toObject();
        server.url = root.value(QStringLiteral("url")).toString();
        server.headers = root.value(QStringLiteral("headers")).toObject();
        server.timeoutMs = root.value(QStringLiteral("timeoutMs")).toInt(server.timeoutMs);
        return server;
    }
};

struct McpServerSnapshot
{
    int schemaVersion = 1;
    QDateTime updatedAt = QDateTime::currentDateTimeUtc();
    QVector<McpServerDefinition> servers;
};

struct McpToolDescriptor
{
    QString name;
    QString description;
    QJsonObject inputSchema;
};

struct McpResourceDescriptor
{
    QString uri;
    QString name;
    QString description;
};

struct McpPromptDescriptor
{
    QString name;
    QString description;
    QJsonArray arguments;
};

struct McpToolCallResult
{
    bool success = false;
    QJsonObject output;
    QString errorCode;
    QString errorMessage;
    bool retryable = false;
};

} // namespace qtllm::tools::mcp
