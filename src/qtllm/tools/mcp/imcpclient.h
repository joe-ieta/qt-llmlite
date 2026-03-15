#pragma once

#include "mcp_types.h"

namespace qtllm::tools {
namespace runtime {
struct ToolExecutionContext;
}

namespace mcp {

class IMcpClient
{
public:
    virtual ~IMcpClient() = default;

    virtual QVector<McpToolDescriptor> listTools(const McpServerDefinition &server,
                                                 QString *errorMessage = nullptr) = 0;

    virtual QVector<McpResourceDescriptor> listResources(const McpServerDefinition &server,
                                                         QString *errorMessage = nullptr) = 0;

    virtual QVector<McpPromptDescriptor> listPrompts(const McpServerDefinition &server,
                                                     QString *errorMessage = nullptr) = 0;

    virtual McpToolCallResult callTool(const McpServerDefinition &server,
                                       const QString &toolName,
                                       const QJsonObject &arguments,
                                       const runtime::ToolExecutionContext &context,
                                       QString *errorMessage = nullptr) = 0;
};

} // namespace mcp
} // namespace qtllm::tools
