#pragma once

#include "defaultmcpclient.h"
#include "mcpserverregistry.h"

#include "../llmtoolregistry.h"

#include <memory>

namespace qtllm::tools::mcp {

class McpToolSyncService
{
public:
    McpToolSyncService(std::shared_ptr<LlmToolRegistry> toolRegistry,
                       std::shared_ptr<McpServerRegistry> serverRegistry,
                       std::shared_ptr<IMcpClient> mcpClient = std::make_shared<DefaultMcpClient>());

    bool syncServerTools(const QString &serverId, QString *errorMessage = nullptr);

private:
    static QString normalizeToolId(const QString &serverId, const QString &toolName);

private:
    std::shared_ptr<LlmToolRegistry> m_toolRegistry;
    std::shared_ptr<McpServerRegistry> m_serverRegistry;
    std::shared_ptr<IMcpClient> m_mcpClient;
};

} // namespace qtllm::tools::mcp
