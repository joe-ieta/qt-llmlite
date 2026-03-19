#pragma once

#include "toolsinside_types.h"

#include <QString>

namespace qtllm::toolsinside {

class ToolsInsideArtifactStore
{
public:
    explicit ToolsInsideArtifactStore(QString rootDirectory = QStringLiteral(".qtllm/tools_inside/artifacts"));

    void setRootDirectory(const QString &rootDirectory);
    QString rootDirectory() const;

    QString absolutePathForRelativePath(const QString &relativePath) const;
    QString traceRelativeDirectory(const QString &clientId,
                                   const QString &sessionId,
                                   const QString &traceId) const;

    ToolsInsideArtifactRef writeArtifact(const QString &clientId,
                                         const QString &sessionId,
                                         const QString &traceId,
                                         const QString &kind,
                                         const QString &mimeType,
                                         const QByteArray &payload,
                                         const IToolsInsideRedactionPolicy &redactionPolicy,
                                         const QJsonObject &metadata = QJsonObject(),
                                         const QString &preferredExtension = QString(),
                                         QString *errorMessage = nullptr) const;

private:
    QString extensionFor(const QString &mimeType, const QString &preferredExtension) const;
    QString sanitizePathComponent(const QString &value) const;
    bool ensureTraceDirectory(const QString &clientId,
                              const QString &sessionId,
                              const QString &traceId,
                              QString *errorMessage = nullptr) const;

private:
    QString m_rootDirectory;
};

} // namespace qtllm::toolsinside
