#pragma once

#include "mcpserverregistry.h"
#include "mcpserverrepository.h"

#include <memory>

namespace qtllm::tools::mcp {

class McpServerManager
{
public:
    explicit McpServerManager(std::shared_ptr<McpServerRegistry> registry = std::make_shared<McpServerRegistry>(),
                              std::shared_ptr<McpServerRepository> repository = std::make_shared<McpServerRepository>());

    bool load(QString *errorMessage = nullptr);
    bool save(QString *errorMessage = nullptr) const;

    bool registerServer(const McpServerDefinition &server, QString *errorMessage = nullptr);
    bool updateServer(const McpServerDefinition &server, QString *errorMessage = nullptr);
    bool removeServer(const QString &serverId, QString *errorMessage = nullptr);

    QVector<McpServerDefinition> allServers() const;
    std::optional<McpServerDefinition> find(const QString &serverId) const;

    std::shared_ptr<McpServerRegistry> registry() const;

private:
    std::shared_ptr<McpServerRegistry> m_registry;
    std::shared_ptr<McpServerRepository> m_repository;
};

} // namespace qtllm::tools::mcp
