#include "toolsinsideartifactstore.h"

#include <QCryptographicHash>
#include <QDir>
#include <QRegularExpression>
#include <QSaveFile>
#include <QUuid>

namespace qtllm::toolsinside {

ToolsInsideArtifactStore::ToolsInsideArtifactStore(QString rootDirectory)
    : m_rootDirectory(std::move(rootDirectory))
{
}

void ToolsInsideArtifactStore::setRootDirectory(const QString &rootDirectory)
{
    m_rootDirectory = rootDirectory.trimmed().isEmpty()
        ? QStringLiteral(".qtllm/tools_inside/artifacts")
        : rootDirectory.trimmed();
}

QString ToolsInsideArtifactStore::rootDirectory() const
{
    return m_rootDirectory;
}

QString ToolsInsideArtifactStore::absolutePathForRelativePath(const QString &relativePath) const
{
    return QDir(m_rootDirectory).filePath(relativePath.trimmed());
}

QString ToolsInsideArtifactStore::traceRelativeDirectory(const QString &clientId,
                                                        const QString &sessionId,
                                                        const QString &traceId) const
{
    return sanitizePathComponent(clientId)
        + QLatin1Char('/')
        + sanitizePathComponent(sessionId)
        + QLatin1Char('/')
        + sanitizePathComponent(traceId);
}

ToolsInsideArtifactRef ToolsInsideArtifactStore::writeArtifact(const QString &clientId,
                                                               const QString &sessionId,
                                                               const QString &traceId,
                                                               const QString &kind,
                                                               const QString &mimeType,
                                                               const QByteArray &payload,
                                                               const IToolsInsideRedactionPolicy &redactionPolicy,
                                                               const QJsonObject &metadata,
                                                               const QString &preferredExtension,
                                                               QString *errorMessage) const
{
    ToolsInsideArtifactRef artifact;
    artifact.artifactId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    artifact.clientId = clientId.trimmed();
    artifact.sessionId = sessionId.trimmed();
    artifact.traceId = traceId.trimmed();
    artifact.kind = kind.trimmed();
    artifact.mimeType = mimeType.trimmed().isEmpty() ? QStringLiteral("application/octet-stream") : mimeType.trimmed();
    artifact.metadata = metadata;

    if (artifact.clientId.isEmpty() || artifact.sessionId.isEmpty() || artifact.traceId.isEmpty() || artifact.kind.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Artifact identifiers are incomplete");
        }
        artifact.artifactId.clear();
        return artifact;
    }

    if (!ensureTraceDirectory(artifact.clientId, artifact.sessionId, artifact.traceId, errorMessage)) {
        artifact.artifactId.clear();
        return artifact;
    }

    const QByteArray persisted = redactionPolicy.redact(artifact.kind, payload, metadata);
    artifact.redactionState = persisted == payload ? QStringLiteral("raw") : QStringLiteral("redacted");
    artifact.sizeBytes = persisted.size();
    artifact.sha256 = QString::fromUtf8(QCryptographicHash::hash(persisted, QCryptographicHash::Sha256).toHex());

    const QString extension = extensionFor(artifact.mimeType, preferredExtension);
    const QString relativeDirectory = traceRelativeDirectory(artifact.clientId, artifact.sessionId, artifact.traceId);
    artifact.relativePath = relativeDirectory + QLatin1Char('/') + artifact.artifactId + extension;

    QSaveFile file(QDir(m_rootDirectory).filePath(artifact.relativePath));
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open artifact for writing: ") + file.errorString();
        }
        artifact.artifactId.clear();
        artifact.relativePath.clear();
        return artifact;
    }

    if (file.write(persisted) != persisted.size()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to write artifact: ") + file.errorString();
        }
        artifact.artifactId.clear();
        artifact.relativePath.clear();
        return artifact;
    }

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to commit artifact: ") + file.errorString();
        }
        artifact.artifactId.clear();
        artifact.relativePath.clear();
        return artifact;
    }

    return artifact;
}

QString ToolsInsideArtifactStore::extensionFor(const QString &mimeType, const QString &preferredExtension) const
{
    if (!preferredExtension.trimmed().isEmpty()) {
        const QString ext = preferredExtension.trimmed();
        return ext.startsWith(QLatin1Char('.')) ? ext : QStringLiteral(".") + ext;
    }

    if (mimeType == QStringLiteral("application/json")) {
        return QStringLiteral(".json");
    }
    if (mimeType.startsWith(QStringLiteral("text/"))) {
        return QStringLiteral(".txt");
    }
    return QStringLiteral(".bin");
}

QString ToolsInsideArtifactStore::sanitizePathComponent(const QString &value) const
{
    QString sanitized = value.trimmed();
    sanitized.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]")), QStringLiteral("_"));
    return sanitized.isEmpty() ? QStringLiteral("default") : sanitized;
}

bool ToolsInsideArtifactStore::ensureTraceDirectory(const QString &clientId,
                                                    const QString &sessionId,
                                                    const QString &traceId,
                                                    QString *errorMessage) const
{
    QDir root(m_rootDirectory);
    const QString relativeDirectory = traceRelativeDirectory(clientId, sessionId, traceId);
    if (root.mkpath(relativeDirectory)) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Failed to create artifact directory: ") + root.filePath(relativeDirectory);
    }
    return false;
}

} // namespace qtllm::toolsinside
