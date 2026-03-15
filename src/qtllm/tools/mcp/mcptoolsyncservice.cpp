#include "mcptoolsyncservice.h"

#include "../../logging/qtllmlogger.h"

#include <QCryptographicHash>
#include <QJsonObject>
#include <QRegularExpression>

namespace qtllm::tools::mcp {

namespace {

QString sanitizeToolName(QString value)
{
    value = value.trimmed().toLower();
    value.replace(QRegularExpression(QStringLiteral("[^a-z0-9_-]")), QStringLiteral("_"));
    return value;
}

QString makeInvocationName(const QString &serverId, const QString &toolName)
{
    const QString prefix = QStringLiteral("mcp_") + sanitizeToolName(serverId) + QStringLiteral("_");
    const QString sanitizedToolName = sanitizeToolName(toolName);
    QString candidate = prefix + sanitizedToolName;
    if (candidate.size() <= 64) {
        return candidate;
    }

    const QByteArray hash = QCryptographicHash::hash((serverId + QStringLiteral("::") + toolName).toUtf8(),
                                                     QCryptographicHash::Sha1).toHex().left(8);
    const int maxBaseLength = qMax(1, 64 - prefix.size() - 1 - hash.size());
    candidate = prefix + sanitizedToolName.left(maxBaseLength) + QStringLiteral("_") + QString::fromLatin1(hash);
    return candidate.left(64);
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
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.sync"),
                                               QStringLiteral("MCP tool sync failed because service configuration is incomplete"),
                                               {},
                                               QJsonObject{{QStringLiteral("serverId"), serverId}});
        return false;
    }

    const std::optional<McpServerDefinition> serverOpt = m_serverRegistry->find(serverId);
    if (!serverOpt.has_value()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server not found: ") + serverId;
        }
        logging::QtLlmLogger::instance().warn(QStringLiteral("mcp.sync"),
                                              QStringLiteral("MCP tool sync skipped because server was not found"),
                                              {},
                                              QJsonObject{{QStringLiteral("serverId"), serverId}});
        return false;
    }

    const McpServerDefinition &server = *serverOpt;
    if (!server.enabled) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("MCP server is disabled: ") + server.serverId;
        }
        logging::QtLlmLogger::instance().info(QStringLiteral("mcp.sync"),
                                              QStringLiteral("MCP tool sync skipped for disabled server"),
                                              {},
                                              QJsonObject{{QStringLiteral("serverId"), server.serverId}});
        return false;
    }

    QString listError;
    const QVector<McpToolDescriptor> tools = m_mcpClient->listTools(server, &listError);
    if (!listError.isEmpty() && tools.isEmpty()) {
        if (errorMessage) {
            *errorMessage = listError;
        }
        logging::QtLlmLogger::instance().error(QStringLiteral("mcp.sync"),
                                               QStringLiteral("MCP tool sync failed during tools/list"),
                                               {},
                                               QJsonObject{{QStringLiteral("serverId"), server.serverId},
                                                           {QStringLiteral("error"), listError}});
        return false;
    }

    int registeredCount = 0;
    int rejectedCount = 0;
    for (const McpToolDescriptor &tool : tools) {
        if (tool.name.trimmed().isEmpty()) {
            ++rejectedCount;
            continue;
        }

        LlmToolDefinition def;
        def.toolId = normalizeToolId(server.serverId, tool.name);
        def.invocationName = makeInvocationName(server.serverId, tool.name);
        def.name = tool.name.trimmed();
        def.description = tool.description.trimmed().isEmpty()
            ? QStringLiteral("MCP tool from server ") + server.serverId
            : tool.description;
        def.inputSchema = tool.inputSchema;
        def.category = QStringLiteral("mcp");
        def.systemBuiltIn = false;
        def.removable = true;
        def.enabled = true;

        def.capabilityTags.append(QStringLiteral("source:mcp"));
        def.capabilityTags.append(QStringLiteral("mcp_server:") + server.serverId);
        def.capabilityTags.append(QStringLiteral("mcp_tool:") + sanitizeToolName(tool.name));

        if (m_toolRegistry->registerTool(def)) {
            ++registeredCount;
        } else {
            ++rejectedCount;
        }
    }

    logging::QtLlmLogger::instance().info(QStringLiteral("mcp.sync"),
                                          QStringLiteral("MCP tool sync completed"),
                                          {},
                                          QJsonObject{{QStringLiteral("serverId"), server.serverId},
                                                      {QStringLiteral("listedCount"), tools.size()},
                                                      {QStringLiteral("registeredCount"), registeredCount},
                                                      {QStringLiteral("rejectedCount"), rejectedCount}});
    return true;
}

QString McpToolSyncService::normalizeToolId(const QString &serverId, const QString &toolName)
{
    return QStringLiteral("mcp::") + serverId.trimmed().toLower() + QStringLiteral("::") + sanitizeToolName(toolName);
}

} // namespace qtllm::tools::mcp
