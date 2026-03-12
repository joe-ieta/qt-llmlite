#pragma once

#include "imcpclient.h"

#include <QNetworkAccessManager>

namespace qtllm::tools::mcp {

class DefaultMcpClient : public IMcpClient
{
public:
    DefaultMcpClient();

    QVector<McpToolDescriptor> listTools(const McpServerDefinition &server,
                                         QString *errorMessage = nullptr) override;

    QVector<McpResourceDescriptor> listResources(const McpServerDefinition &server,
                                                 QString *errorMessage = nullptr) override;

    QVector<McpPromptDescriptor> listPrompts(const McpServerDefinition &server,
                                             QString *errorMessage = nullptr) override;

    McpToolCallResult callTool(const McpServerDefinition &server,
                               const QString &toolName,
                               const QJsonObject &arguments,
                               const runtime::ToolExecutionContext &context,
                               QString *errorMessage = nullptr) override;

private:
    QJsonObject performJsonRpc(const McpServerDefinition &server,
                               const QString &method,
                               const QJsonObject &params,
                               QString *errorMessage) const;

    QJsonObject performStdioJsonRpc(const McpServerDefinition &server,
                                    const QJsonObject &request,
                                    QString *errorMessage) const;

    QJsonObject performHttpLikeJsonRpc(const McpServerDefinition &server,
                                       const QJsonObject &request,
                                       QString *errorMessage) const;

private:
    mutable QNetworkAccessManager m_network;
    mutable int m_requestIdSeed = 1;
};

} // namespace qtllm::tools::mcp
