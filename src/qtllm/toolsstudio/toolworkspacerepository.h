#pragma once

#include "toolsstudio_types.h"

#include <optional>

namespace qtllm::toolsstudio {

class ToolWorkspaceRepository
{
public:
    explicit ToolWorkspaceRepository(QString rootDirectory = QStringLiteral(".qtllm/tools/studio"));

    bool saveWorkspace(const ToolWorkspaceSnapshot &workspace, QString *errorMessage = nullptr) const;
    std::optional<ToolWorkspaceSnapshot> loadWorkspace(const QString &workspaceId, QString *errorMessage = nullptr) const;
    bool deleteWorkspace(const QString &workspaceId, QString *errorMessage = nullptr) const;

    bool saveIndex(const ToolWorkspaceIndex &index, QString *errorMessage = nullptr) const;
    std::optional<ToolWorkspaceIndex> loadIndex(QString *errorMessage = nullptr) const;

    QString workspacePath(const QString &workspaceId) const;
    QString indexPath() const;

private:
    bool ensureRootDirectory(QString *errorMessage = nullptr) const;
    bool ensureWorkspaceDirectory(QString *errorMessage = nullptr) const;
    QString sanitizeWorkspaceId(const QString &workspaceId) const;

private:
    QString m_rootDirectory;
};

} // namespace qtllm::toolsstudio
