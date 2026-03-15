#include "mcpserverrepository.h"

#include "../../logging/qtllmlogger.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>

namespace qtllm::tools::mcp {

McpServerRepository::McpServerRepository(QString rootDirectory)
    : m_rootDirectory(std::move(rootDirectory))
{
}

bool McpServerRepository::saveServers(const McpServerSnapshot &snapshot, QString *errorMessage) const
{
    if (!ensureRootDirectory(errorMessage)) {
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.repository"),
                                               QStringLiteral("Failed to ensure MCP repository directory before save"),
                                               {},
                                               QJsonObject{{QStringLiteral("rootDirectory"), m_rootDirectory}});
        return false;
    }

    QSaveFile file(serverListPath());
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open MCP server list for writing: ") + file.errorString();
        }
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.repository"),
                                               QStringLiteral("Failed to open MCP server list for writing"),
                                               {},
                                               QJsonObject{{QStringLiteral("path"), serverListPath()},
                                                           {QStringLiteral("error"), file.errorString()}});
        return false;
    }

    QJsonObject root;
    root.insert(QStringLiteral("schemaVersion"), snapshot.schemaVersion);
    root.insert(QStringLiteral("updatedAt"), snapshot.updatedAt.toString(Qt::ISODateWithMs));

    QJsonArray serverArray;
    for (const McpServerDefinition &server : snapshot.servers) {
        serverArray.append(server.toJson());
    }
    root.insert(QStringLiteral("servers"), serverArray);

    const QJsonDocument doc(root);
    if (file.write(doc.toJson(QJsonDocument::Indented)) < 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to write MCP server list: ") + file.errorString();
        }
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.repository"),
                                               QStringLiteral("Failed to write MCP server list"),
                                               {},
                                               QJsonObject{{QStringLiteral("path"), serverListPath()},
                                                           {QStringLiteral("error"), file.errorString()}});
        return false;
    }

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to commit MCP server list: ") + file.errorString();
        }
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.repository"),
                                               QStringLiteral("Failed to commit MCP server list"),
                                               {},
                                               QJsonObject{{QStringLiteral("path"), serverListPath()},
                                                           {QStringLiteral("error"), file.errorString()}});
        return false;
    }

    logging::QtLlmLogger::instance().info(QStringLiteral("mcp.repository"),
                                          QStringLiteral("MCP server list saved"),
                                          {},
                                          QJsonObject{{QStringLiteral("path"), serverListPath()},
                                                      {QStringLiteral("serverCount"), snapshot.servers.size()}});
    return true;
}

std::optional<McpServerSnapshot> McpServerRepository::loadServers(QString *errorMessage) const
{
    QFile file(serverListPath());
    if (!file.exists()) {
        logging::QtLlmLogger::instance().debug(QStringLiteral("mcp.repository"),
                                               QStringLiteral("MCP server list does not exist yet"),
                                               {},
                                               QJsonObject{{QStringLiteral("path"), serverListPath()}});
        return std::nullopt;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open MCP server list for reading: ") + file.errorString();
        }
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.repository"),
                                               QStringLiteral("Failed to open MCP server list for reading"),
                                               {},
                                               QJsonObject{{QStringLiteral("path"), serverListPath()},
                                                           {QStringLiteral("error"), file.errorString()}});
        return std::nullopt;
    }

    const QByteArray payload = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server list JSON parse error: ") + parseError.errorString();
        }
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.repository"),
                                               QStringLiteral("MCP server list JSON parse error"),
                                               {},
                                               QJsonObject{{QStringLiteral("path"), serverListPath()},
                                                           {QStringLiteral("error"), parseError.errorString()}});
        return std::nullopt;
    }

    McpServerSnapshot snapshot;
    const QJsonObject root = doc.object();
    snapshot.schemaVersion = root.value(QStringLiteral("schemaVersion")).toInt(snapshot.schemaVersion);

    const QDateTime updatedAt = QDateTime::fromString(root.value(QStringLiteral("updatedAt")).toString(),
                                                       Qt::ISODateWithMs);
    if (updatedAt.isValid()) {
        snapshot.updatedAt = updatedAt;
    }

    int skipped = 0;
    const QJsonArray serverArray = root.value(QStringLiteral("servers")).toArray();
    for (const QJsonValue &value : serverArray) {
        if (!value.isObject()) {
            ++skipped;
            continue;
        }

        const McpServerDefinition server = McpServerDefinition::fromJson(value.toObject());
        if (!server.serverId.trimmed().isEmpty()) {
            snapshot.servers.append(server);
        } else {
            ++skipped;
        }
    }

    logging::QtLlmLogger::instance().info(QStringLiteral("mcp.repository"),
                                          QStringLiteral("MCP server list loaded"),
                                          {},
                                          QJsonObject{{QStringLiteral("path"), serverListPath()},
                                                      {QStringLiteral("serverCount"), snapshot.servers.size()},
                                                      {QStringLiteral("skippedCount"), skipped}});
    return snapshot;
}

QString McpServerRepository::serverListPath() const
{
    return QDir(m_rootDirectory).filePath(QStringLiteral("servers.json"));
}

bool McpServerRepository::ensureRootDirectory(QString *errorMessage) const
{
    QDir dir(m_rootDirectory);
    if (dir.exists()) {
        return true;
    }

    if (dir.mkpath(QStringLiteral("."))) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Failed to create MCP directory: ") + m_rootDirectory;
    }
    return false;
}

} // namespace qtllm::tools::mcp
