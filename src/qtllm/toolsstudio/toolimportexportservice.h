#pragma once

#include "toolsstudio_types.h"

#include <QSet>

#include <optional>

namespace qtllm::toolsstudio {

class ToolImportExportService
{
public:
    bool exportWorkspace(const ToolWorkspaceSnapshot &workspace,
                         const QList<qtllm::tools::LlmToolDefinition> &catalogTools,
                         const QString &filePath,
                         bool includeToolSnapshots,
                         QString *errorMessage = nullptr) const;

    bool exportSubtree(const ToolWorkspaceSnapshot &workspace,
                       const QString &rootNodeId,
                       const QList<qtllm::tools::LlmToolDefinition> &catalogTools,
                       const QString &filePath,
                       bool includeToolSnapshots,
                       QString *errorMessage = nullptr) const;

    std::optional<ToolImportPackage> loadPackage(const QString &filePath, QString *errorMessage = nullptr) const;

private:
    ToolImportPackage buildPackage(const ToolWorkspaceSnapshot &workspace,
                                   const QList<qtllm::tools::LlmToolDefinition> &catalogTools,
                                   bool includeToolSnapshots,
                                   const QString &exportMode) const;
    bool savePackage(const ToolImportPackage &package, const QString &filePath, QString *errorMessage) const;
    QSet<QString> subtreeNodeIds(const ToolWorkspaceSnapshot &workspace, const QString &rootNodeId) const;
};

} // namespace qtllm::toolsstudio
