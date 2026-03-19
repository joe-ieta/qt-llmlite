#include "toolworkspacerepository.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QSaveFile>

namespace qtllm::toolsstudio {

ToolWorkspaceRepository::ToolWorkspaceRepository(QString rootDirectory)
    : m_rootDirectory(std::move(rootDirectory))
{
}

bool ToolWorkspaceRepository::saveWorkspace(const ToolWorkspaceSnapshot &workspace, QString *errorMessage) const
{
    if (!ensureWorkspaceDirectory(errorMessage)) {
        return false;
    }

    QSaveFile file(workspacePath(workspace.workspaceId));
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open workspace for writing: ") + file.errorString();
        }
        return false;
    }

    const QJsonDocument doc(workspace.toJson());
    if (file.write(doc.toJson(QJsonDocument::Indented)) < 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to write workspace: ") + file.errorString();
        }
        return false;
    }

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to commit workspace: ") + file.errorString();
        }
        return false;
    }

    return true;
}

std::optional<ToolWorkspaceSnapshot> ToolWorkspaceRepository::loadWorkspace(const QString &workspaceId,
                                                                            QString *errorMessage) const
{
    QFile file(workspacePath(workspaceId));
    if (!file.exists()) {
        return std::nullopt;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open workspace for reading: ") + file.errorString();
        }
        return std::nullopt;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Workspace JSON parse error: ") + parseError.errorString();
        }
        return std::nullopt;
    }

    return ToolWorkspaceSnapshot::fromJson(doc.object());
}

bool ToolWorkspaceRepository::deleteWorkspace(const QString &workspaceId, QString *errorMessage) const
{
    QFile file(workspacePath(workspaceId));
    if (!file.exists()) {
        return true;
    }

    if (!file.remove()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to delete workspace: ") + file.errorString();
        }
        return false;
    }

    return true;
}

bool ToolWorkspaceRepository::saveIndex(const ToolWorkspaceIndex &index, QString *errorMessage) const
{
    if (!ensureRootDirectory(errorMessage)) {
        return false;
    }

    QSaveFile file(indexPath());
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open workspace index for writing: ") + file.errorString();
        }
        return false;
    }

    const QJsonDocument doc(index.toJson());
    if (file.write(doc.toJson(QJsonDocument::Indented)) < 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to write workspace index: ") + file.errorString();
        }
        return false;
    }

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to commit workspace index: ") + file.errorString();
        }
        return false;
    }

    return true;
}

std::optional<ToolWorkspaceIndex> ToolWorkspaceRepository::loadIndex(QString *errorMessage) const
{
    QFile file(indexPath());
    if (!file.exists()) {
        return std::nullopt;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open workspace index for reading: ") + file.errorString();
        }
        return std::nullopt;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Workspace index JSON parse error: ") + parseError.errorString();
        }
        return std::nullopt;
    }

    return ToolWorkspaceIndex::fromJson(doc.object());
}

QString ToolWorkspaceRepository::workspacePath(const QString &workspaceId) const
{
    return QDir(m_rootDirectory).filePath(QStringLiteral("workspaces/") + sanitizeWorkspaceId(workspaceId) + QStringLiteral(".json"));
}

QString ToolWorkspaceRepository::indexPath() const
{
    return QDir(m_rootDirectory).filePath(QStringLiteral("workspace_index.json"));
}

bool ToolWorkspaceRepository::ensureRootDirectory(QString *errorMessage) const
{
    QDir dir(m_rootDirectory);
    if (dir.exists() || dir.mkpath(QStringLiteral("."))) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Failed to create root directory: ") + m_rootDirectory;
    }
    return false;
}

bool ToolWorkspaceRepository::ensureWorkspaceDirectory(QString *errorMessage) const
{
    if (!ensureRootDirectory(errorMessage)) {
        return false;
    }

    QDir dir(QDir(m_rootDirectory).filePath(QStringLiteral("workspaces")));
    if (dir.exists() || dir.mkpath(QStringLiteral("."))) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Failed to create workspace directory");
    }
    return false;
}

QString ToolWorkspaceRepository::sanitizeWorkspaceId(const QString &workspaceId) const
{
    QString value = workspaceId.trimmed().toLower();
    if (value.isEmpty()) {
        value = QStringLiteral("default");
    }
    value.replace(QRegularExpression(QStringLiteral("[^a-z0-9_-]")), QStringLiteral("_"));
    return value;
}

} // namespace qtllm::toolsstudio
