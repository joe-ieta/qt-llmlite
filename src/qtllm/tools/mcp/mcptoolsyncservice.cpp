#include "mcptoolsyncservice.h"

#include <QRegularExpression>

namespace qtllm::tools::mcp {

namespace {

QString sanitizeToolName(QString value)
{
    value = value.trimmed().toLower();
    value.replace(QRegularExpression(QStringLiteral("[^a-z0-9_-]")), QStringLiteral("_"));
    return value;
}

} // namespace

McpToolSyncService::McpToolSyncService(std::shared_ptr<LlmToolRegistry> toolRegistry,
                                       std::shared_ptr<McpServerRegistry> serverRegistry,
                                       std::shared_ptr<IMcpClient> mcpClient)
    : m_toolRegistry(std::move(toolRegistry))
    , m_serverRegistry(std::move(serverRegistry))
    , m_mcpClient(std::move(mcpClient))
{
}

bool McpToolSyncService::syncServerTools(const QString &serverId, QString *errorMessage)
{
    if (!m_toolRegistry || !m_serverRegistry || !m_mcpClient) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP sync service is not fully configured");
        }
        return false;
    }

    const std::optional<McpServerDefinition> serverOpt = m_serverRegistry->find(serverId);
    if (!serverOpt.has_value()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server not found: ") + serverId;
        }
        return false;
    }

    const McpServerDefinition &server = *serverOpt;
    if (!server.enabled) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server is disabled: ") + server.serverId;
        }
        return false;
    }

    QString listError;
    const QVector<McpToolDescriptor> tools = m_mcpClient->listTools(server, &listError);
    if (!listError.isEmpty() && tools.isEmpty()) {
        if (errorMessage) {
            *errorMessage = listError;
        }
        return false;
    }

    for (const McpToolDescriptor &tool : tools) {
        if (tool.name.trimmed().isEmpty()) {
            continue;
        }

        LlmToolDefinition def;
        def.toolId = normalizeToolId(server.serverId, tool.name);
        def.name = def.toolId;
        def.description = tool.description.trimmed().isEmpty()
            ? QStringLiteral("MCP tool from server ") + server.serverId
            : tool.description;
        def.inputSchema = tool.inputSchema;
        def.category = QStringLiteral("mcp");
        def.systemBuiltIn = false;
        def.removable = true;
        def.enabled = true;

        // Carry source metadata with lightweight tags for now.
        def.capabilityTags.append(QStringLiteral("source:mcp"));
        def.capabilityTags.append(QStringLiteral("mcp_server:") + server.serverId);
        def.capabilityTags.append(QStringLiteral("mcp_tool:") + sanitizeToolName(tool.name));

        m_toolRegistry->registerTool(def);
    }

    return true;
}

QString McpToolSyncService::normalizeToolId(const QString &serverId, const QString &toolName)
{
    return QStringLiteral("mcp::") + serverId.trimmed().toLower() + QStringLiteral("::") + sanitizeToolName(toolName);
}

} // namespace qtllm::tools::mcp
