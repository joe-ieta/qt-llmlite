#include "mcpservermanager.h"

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
        return false;
    }

    const std::optional<McpServerSnapshot> snapshot = m_repository->loadServers(errorMessage);
    if (!snapshot.has_value()) {
        return true;
    }

    m_registry->replaceAll(snapshot->servers, errorMessage);
    return true;
}

bool McpServerManager::save(QString *errorMessage) const
{
    if (!m_registry || !m_repository) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server manager is not fully configured");
        }
        return false;
    }

    McpServerSnapshot snapshot;
    snapshot.servers = m_registry->allServers();
    return m_repository->saveServers(snapshot, errorMessage);
}

bool McpServerManager::registerServer(const McpServerDefinition &server, QString *errorMessage)
{
    if (!m_registry) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server registry is not configured");
        }
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
