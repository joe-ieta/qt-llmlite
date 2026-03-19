#pragma once

#include "toolsinside_types.h"

#include <QMutex>
#include <QString>

#include <optional>

namespace qtllm::toolsinside {

class ToolsInsideRepository
{
public:
    explicit ToolsInsideRepository(QString databasePath = QStringLiteral(".qtllm/tools_inside/index.db"));
    ~ToolsInsideRepository();

    void setDatabasePath(const QString &databasePath);
    QString databasePath() const;

    bool ensureInitialized(QString *errorMessage = nullptr) const;

    bool upsertTrace(const ToolsInsideTraceSummary &trace, QString *errorMessage = nullptr) const;
    bool finishTrace(const QString &traceId,
                     const QString &status,
                     const QString &finalAnswerArtifactId = QString(),
                     const QDateTime &endedAtUtc = QDateTime::currentDateTimeUtc(),
                     QString *errorMessage = nullptr) const;
    bool updateTraceStatus(const QString &traceId,
                           const QString &status,
                           QString *errorMessage = nullptr) const;

    bool upsertSpan(const ToolsInsideSpanRecord &span, QString *errorMessage = nullptr) const;
    bool upsertEvent(const ToolsInsideEventRecord &event, QString *errorMessage = nullptr) const;
    bool upsertArtifact(const ToolsInsideArtifactRef &artifact, QString *errorMessage = nullptr) const;
    bool updateArtifactPathPrefix(const QString &traceId,
                                  const QString &oldPrefix,
                                  const QString &newPrefix,
                                  QString *errorMessage = nullptr) const;
    bool upsertToolCall(const ToolsInsideToolCallRecord &toolCall, QString *errorMessage = nullptr) const;
    bool upsertSupportLink(const ToolsInsideSupportLink &supportLink, QString *errorMessage = nullptr) const;
    bool deleteTrace(const QString &traceId, QString *errorMessage = nullptr) const;

    QList<ToolsInsideClientSummary> listClients(QString *errorMessage = nullptr) const;
    QList<ToolsInsideSessionSummary> listSessions(const QString &clientId,
                                                  QString *errorMessage = nullptr) const;
    QList<ToolsInsideTraceSummary> listTraces(const ToolsInsideTraceFilter &filter,
                                              QString *errorMessage = nullptr) const;
    std::optional<ToolsInsideTraceSummary> getTrace(const QString &traceId,
                                                    QString *errorMessage = nullptr) const;
    QList<ToolsInsideEventRecord> listEvents(const QString &traceId,
                                             QString *errorMessage = nullptr) const;
    QList<ToolsInsideSpanRecord> listSpans(const QString &traceId,
                                           QString *errorMessage = nullptr) const;
    QList<ToolsInsideArtifactRef> listArtifacts(const QString &traceId,
                                                QString *errorMessage = nullptr) const;
    QList<ToolsInsideToolCallRecord> listToolCalls(const QString &traceId,
                                                   QString *errorMessage = nullptr) const;
    QList<ToolsInsideSupportLink> listSupportLinks(const QString &traceId,
                                                   QString *errorMessage = nullptr) const;
    ToolsInsideStorageStats getStorageStats(QString *errorMessage = nullptr) const;

private:
    bool ensureConnectionLocked(QString *errorMessage = nullptr) const;
    bool runMigrationsLocked(QString *errorMessage = nullptr) const;

private:
    QString m_databasePath;
    QString m_connectionName;
    mutable QMutex m_mutex;
};

} // namespace qtllm::toolsinside
