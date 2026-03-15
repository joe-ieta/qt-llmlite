#pragma once

#include "mcp_types.h"

#include <QHash>

#include <optional>

namespace qtllm::tools::mcp {

class McpServerRegistry
{
public:
    bool registerServer(const McpServerDefinition &server, QString *errorMessage = nullptr);
    bool updateServer(const McpServerDefinition &server, QString *errorMessage = nullptr);
    bool removeServer(const QString &serverId, QString *errorMessage = nullptr);

    bool contains(const QString &serverId) const;
    std::optional<McpServerDefinition> find(const QString &serverId) const;
    QVector<McpServerDefinition> allServers() const;

    void clear();
    void replaceAll(const QVector<McpServerDefinition> &servers, QString *errorMessage = nullptr);

    static bool isValidServerId(const QString &serverId);

private:
    QHash<QString, McpServerDefinition> m_servers;
};

} // namespace qtllm::tools::mcp
