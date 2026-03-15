#include "mcpservermanager.h"

#include "../../logging/qtllmlogger.h"

#include <QJsonObject>

namespace qtllm::tools::mcp {

McpServerManager::McpServerManager(std::shared_ptr<McpServerRegistry> registry,
                                   std::shared_ptr<McpServerRepository> repository)
    : m_registry(std::move(registry))
    , m_repository(std::move(repository))
{
}

bool McpServerManager::load(QString *errorMessage)
{
    if (!m_registry || !m_repository) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server manager is not fully configured");
        }
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.manager"),
                                               QStringLiteral("MCP server manager load failed because configuration is incomplete"));
        return false;
    }

    const std::optional<McpServerSnapshot> snapshot = m_repository->loadServers(errorMessage);
    if (!snapshot.has_value()) {
        logging::QtLlmLogger::instance().debug(QStringLiteral("mcp.manager"),
                                               QStringLiteral("No persisted MCP server snapshot was loaded"));
        return true;
    }

    m_registry->replaceAll(snapshot->servers, errorMessage);
    logging::QtLlmLogger::instance().info(QStringLiteral("mcp.manager"),
                                          QStringLiteral("Persisted MCP servers loaded into registry"),
                                          {},
                                          QJsonObject{{QStringLiteral("serverCount"), snapshot->servers.size()}});
    return true;
}

bool McpServerManager::save(QString *errorMessage) const
{
    if (!m_registry || !m_repository) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server manager is not fully configured");
        }
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.manager"),
                                               QStringLiteral("MCP server manager save failed because configuration is incomplete"));
        return false;
    }

    McpServerSnapshot snapshot;
    snapshot.servers = m_registry->allServers();
    const bool ok = m_repository->saveServers(snapshot, errorMessage);
    logging::QtLlmLogger::instance().info(QStringLiteral("mcp.manager"),
                                          ok
                                              ? QStringLiteral("Persisted MCP servers saved")
                                              : QStringLiteral("Persisted MCP servers save failed"),
                                          {},
                                          QJsonObject{{QStringLiteral("serverCount"), snapshot.servers.size()}});
    return ok;
}

bool McpServerManager::registerServer(const McpServerDefinition &server, QString *errorMessage)
{
    if (!m_registry) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server registry is not configured");
        }
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.manager"),
                                               QStringLiteral("MCP server registration failed because registry is missing"));
        return false;
    }

    if (!m_registry->registerServer(server, errorMessage)) {
        return false;
    }

    return save(errorMessage);
}

bool McpServerManager::updateServer(const McpServerDefinition &server, QString *errorMessage)
{
    if (!m_registry) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server registry is not configured");
        }
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.manager"),
                                               QStringLiteral("MCP server update failed because registry is missing"));
        return false;
    }

    if (!m_registry->updateServer(server, errorMessage)) {
        return false;
    }

    return save(errorMessage);
}

bool McpServerManager::removeServer(const QString &serverId, QString *errorMessage)
{
    if (!m_registry) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server registry is not configured");
        }
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.manager"),
                                               QStringLiteral("MCP server removal failed because registry is missing"));
        return false;
    }

    if (!m_registry->removeServer(serverId, errorMessage)) {
        return false;
    }

    return save(errorMessage);
}

QVector<McpServerDefinition> McpServerManager::allServers() const
{
    return m_registry ? m_registry->allServers() : QVector<McpServerDefinition>();
}

std::optional<McpServerDefinition> McpServerManager::find(const QString &serverId) const
{
    return m_registry ? m_registry->find(serverId) : std::nullopt;
}

std::shared_ptr<McpServerRegistry> McpServerManager::registry() const
{
    return m_registry;
}

} // namespace qtllm::tools::mcp
