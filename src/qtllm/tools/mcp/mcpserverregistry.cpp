#include "mcpserverregistry.h"

#include <QRegularExpression>

namespace qtllm::tools::mcp {

namespace {

QString normalizeId(const QString &serverId)
{
    return serverId.trimmed().toLower();
}

bool validateServer(const McpServerDefinition &server, QString *errorMessage)
{
    const QString id = normalizeId(server.serverId);
    if (!McpServerRegistry::isValidServerId(id)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid serverId. Use 1-32 chars: [a-z0-9_-]");
        }
        return false;
    }

    const QString transport = server.transport.trimmed().toLower();
    if (transport != QStringLiteral("stdio")
        && transport != QStringLiteral("streamable-http")
        && transport != QStringLiteral("sse")) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Unsupported transport: ") + server.transport;
        }
        return false;
    }

    if (transport == QStringLiteral("stdio") && server.command.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("stdio transport requires command");
        }
        return false;
    }

    if ((transport == QStringLiteral("streamable-http") || transport == QStringLiteral("sse"))
        && server.url.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("HTTP/SSE transport requires url");
        }
        return false;
    }

    return true;
}

} // namespace

bool McpServerRegistry::registerServer(const McpServerDefinition &server, QString *errorMessage)
{
    if (!validateServer(server, errorMessage)) {
        return false;
    }

    const QString id = normalizeId(server.serverId);
    if (m_servers.contains(id)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server already exists: ") + id;
        }
        return false;
    }

    McpServerDefinition normalized = server;
    normalized.serverId = id;
    normalized.transport = normalized.transport.trimmed().toLower();
    m_servers.insert(id, normalized);
    return true;
}

bool McpServerRegistry::updateServer(const McpServerDefinition &server, QString *errorMessage)
{
    if (!validateServer(server, errorMessage)) {
        return false;
    }

    const QString id = normalizeId(server.serverId);
    if (!m_servers.contains(id)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server does not exist: ") + id;
        }
        return false;
    }

    McpServerDefinition normalized = server;
    normalized.serverId = id;
    normalized.transport = normalized.transport.trimmed().toLower();
    m_servers.insert(id, normalized);
    return true;
}

bool McpServerRegistry::removeServer(const QString &serverId, QString *errorMessage)
{
    const QString id = normalizeId(serverId);
    if (id.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("serverId is empty");
        }
        return false;
    }

    if (m_servers.remove(id) <= 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server does not exist: ") + id;
        }
        return false;
    }

    return true;
}

bool McpServerRegistry::contains(const QString &serverId) const
{
    return m_servers.contains(normalizeId(serverId));
}

std::optional<McpServerDefinition> McpServerRegistry::find(const QString &serverId) const
{
    const QString id = normalizeId(serverId);
    const auto it = m_servers.constFind(id);
    if (it == m_servers.constEnd()) {
        return std::nullopt;
    }
    return *it;
}

QVector<McpServerDefinition> McpServerRegistry::allServers() const
{
    return m_servers.values().toVector();
}

void McpServerRegistry::clear()
{
    m_servers.clear();
}

void McpServerRegistry::replaceAll(const QVector<McpServerDefinition> &servers, QString *errorMessage)
{
    m_servers.clear();
    for (const McpServerDefinition &server : servers) {
        QString localError;
        if (!registerServer(server, &localError)) {
            if (errorMessage) {
                *errorMessage = localError;
            }
        }
    }
}

bool McpServerRegistry::isValidServerId(const QString &serverId)
{
    static const QRegularExpression re(QStringLiteral("^[a-z0-9_-]{1,32}$"));
    return re.match(serverId.trimmed().toLower()).hasMatch();
}

} // namespace qtllm::tools::mcp
