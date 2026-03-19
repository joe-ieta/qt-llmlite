#include "toolsinsideadminservice.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

namespace qtllm::toolsinside {

namespace {

QString sanitizePathComponent(const QString &value)
{
    QString sanitized = value.trimmed();
    sanitized.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]")), QStringLiteral("_"));
    return sanitized.isEmpty() ? QStringLiteral("default") : sanitized;
}

bool removeDirectoryTree(const QString &absolutePath, QString *errorMessage)
{
    QFileInfo info(absolutePath);
    if (!info.exists()) {
        return true;
    }

    QDir dir(absolutePath);
    if (dir.removeRecursively()) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Failed to remove directory: ") + absolutePath;
    }
    return false;
}

} // namespace

ToolsInsideAdminService::ToolsInsideAdminService(QString rootDirectory,
                                                 std::shared_ptr<ToolsInsideRepository> repository,
                                                 std::shared_ptr<ToolsInsideArtifactStore> artifactStore)
    : m_rootDirectory(std::move(rootDirectory))
    , m_repository(std::move(repository))
    , m_artifactStore(std::move(artifactStore))
{
}

void ToolsInsideAdminService::setRootDirectory(const QString &rootDirectory)
{
    m_rootDirectory = rootDirectory.trimmed().isEmpty()
        ? QStringLiteral(".qtllm/tools_inside")
        : rootDirectory.trimmed();
}

QString ToolsInsideAdminService::rootDirectory() const
{
    return m_rootDirectory;
}

void ToolsInsideAdminService::setRepository(const std::shared_ptr<ToolsInsideRepository> &repository)
{
    m_repository = repository;
}

void ToolsInsideAdminService::setArtifactStore(const std::shared_ptr<ToolsInsideArtifactStore> &artifactStore)
{
    m_artifactStore = artifactStore;
}

bool ToolsInsideAdminService::archiveTrace(const QString &traceId, QString *errorMessage) const
{
    if (errorMessage) {
        errorMessage->clear();
    }
    if (!m_repository || !m_artifactStore) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("ToolsInsideAdminService is not fully configured");
        }
        return false;
    }

    const std::optional<ToolsInsideTraceSummary> trace = m_repository->getTrace(traceId, errorMessage);
    if (!trace.has_value()) {
        if (errorMessage && errorMessage->isEmpty()) {
            *errorMessage = QStringLiteral("Trace not found: ") + traceId;
        }
        return false;
    }

    const QString activeRelativeDir = m_artifactStore->traceRelativeDirectory(trace->clientId, trace->sessionId, trace->traceId);
    const QString archiveRelativeDir = QStringLiteral("archive/") + activeRelativeDir;
    const QString activeAbsoluteDir = m_artifactStore->absolutePathForRelativePath(activeRelativeDir);
    const QString archiveAbsoluteDir = m_artifactStore->absolutePathForRelativePath(archiveRelativeDir);

    const QFileInfo activeInfo(activeAbsoluteDir);
    if (activeInfo.exists()) {
        QDir archiveParent(QFileInfo(archiveAbsoluteDir).absolutePath());
        if (!archiveParent.exists() && !archiveParent.mkpath(QStringLiteral("."))) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Failed to create archive directory: ") + archiveParent.path();
            }
            return false;
        }

        if (QFileInfo::exists(archiveAbsoluteDir) && !removeDirectoryTree(archiveAbsoluteDir, errorMessage)) {
            return false;
        }

        QDir dir;
        if (!dir.rename(activeAbsoluteDir, archiveAbsoluteDir)) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Failed to move artifacts to archive: ") + activeAbsoluteDir;
            }
            return false;
        }

        if (!m_repository->updateArtifactPathPrefix(traceId, activeRelativeDir, archiveRelativeDir, errorMessage)) {
            return false;
        }
    }

    return m_repository->updateTraceStatus(traceId, QStringLiteral("archived"), errorMessage);
}

bool ToolsInsideAdminService::purgeTrace(const QString &traceId, QString *errorMessage) const
{
    if (errorMessage) {
        errorMessage->clear();
    }
    if (!m_repository || !m_artifactStore) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("ToolsInsideAdminService is not fully configured");
        }
        return false;
    }

    const std::optional<ToolsInsideTraceSummary> trace = m_repository->getTrace(traceId, errorMessage);
    if (!trace.has_value()) {
        if (errorMessage && errorMessage->isEmpty()) {
            *errorMessage = QStringLiteral("Trace not found: ") + traceId;
        }
        return false;
    }

    const QString activeRelativeDir = m_artifactStore->traceRelativeDirectory(trace->clientId, trace->sessionId, trace->traceId);
    const QString archiveRelativeDir = QStringLiteral("archive/") + activeRelativeDir;
    const QString activeAbsoluteDir = m_artifactStore->absolutePathForRelativePath(activeRelativeDir);
    const QString archiveAbsoluteDir = m_artifactStore->absolutePathForRelativePath(archiveRelativeDir);

    if (!removeDirectoryTree(activeAbsoluteDir, errorMessage)) {
        return false;
    }
    if (!removeDirectoryTree(archiveAbsoluteDir, errorMessage)) {
        return false;
    }

    return m_repository->deleteTrace(traceId, errorMessage);
}

} // namespace qtllm::toolsinside
