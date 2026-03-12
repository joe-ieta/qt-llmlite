#pragma once

#include "mcp_types.h"

#include <optional>

namespace qtllm::tools::mcp {

class McpServerRepository
{
public:
    explicit McpServerRepository(QString rootDirectory = QStringLiteral(".qtllm/mcp"));

    bool saveServers(const McpServerSnapshot &snapshot, QString *errorMessage = nullptr) const;
    std::optional<McpServerSnapshot> loadServers(QString *errorMessage = nullptr) const;

private:
    QString serverListPath() const;
    bool ensureRootDirectory(QString *errorMessage = nullptr) const;

private:
    QString m_rootDirectory;
};

} // namespace qtllm::tools::mcp
