#include "toolsinsidequeryservice.h"

namespace qtllm::toolsinside {

ToolsInsideQueryService::ToolsInsideQueryService(std::shared_ptr<ToolsInsideRepository> repository)
    : m_repository(std::move(repository))
{
}

void ToolsInsideQueryService::setRepository(const std::shared_ptr<ToolsInsideRepository> &repository)
{
    m_repository = repository;
}

QList<ToolsInsideClientSummary> ToolsInsideQueryService::listClients(QString *errorMessage) const
{
    return m_repository ? m_repository->listClients(errorMessage) : QList<ToolsInsideClientSummary>();
}

QList<ToolsInsideSessionSummary> ToolsInsideQueryService::listSessions(const QString &clientId,
                                                                       QString *errorMessage) const
{
    return m_repository ? m_repository->listSessions(clientId, errorMessage) : QList<ToolsInsideSessionSummary>();
}

QList<ToolsInsideTraceSummary> ToolsInsideQueryService::listTraces(const ToolsInsideTraceFilter &filter,
                                                                   QString *errorMessage) const
{
    return m_repository ? m_repository->listTraces(filter, errorMessage) : QList<ToolsInsideTraceSummary>();
}

std::optional<ToolsInsideTraceSummary> ToolsInsideQueryService::getTraceSummary(const QString &traceId,
                                                                                QString *errorMessage) const
{
    return m_repository ? m_repository->getTrace(traceId, errorMessage) : std::optional<ToolsInsideTraceSummary>();
}

QList<ToolsInsideEventRecord> ToolsInsideQueryService::getTraceTimeline(const QString &traceId,
                                                                        QString *errorMessage) const
{
    return m_repository ? m_repository->listEvents(traceId, errorMessage) : QList<ToolsInsideEventRecord>();
}

QList<ToolsInsideSpanRecord> ToolsInsideQueryService::getTraceSpans(const QString &traceId,
                                                                    QString *errorMessage) const
{
    return m_repository ? m_repository->listSpans(traceId, errorMessage) : QList<ToolsInsideSpanRecord>();
}

QList<ToolsInsideArtifactRef> ToolsInsideQueryService::getTraceArtifacts(const QString &traceId,
                                                                         QString *errorMessage) const
{
    return m_repository ? m_repository->listArtifacts(traceId, errorMessage) : QList<ToolsInsideArtifactRef>();
}

QList<ToolsInsideToolCallRecord> ToolsInsideQueryService::getTraceToolCalls(const QString &traceId,
                                                                            QString *errorMessage) const
{
    return m_repository ? m_repository->listToolCalls(traceId, errorMessage) : QList<ToolsInsideToolCallRecord>();
}

QList<ToolsInsideSupportLink> ToolsInsideQueryService::getTraceSupportLinks(const QString &traceId,
                                                                            QString *errorMessage) const
{
    return m_repository ? m_repository->listSupportLinks(traceId, errorMessage) : QList<ToolsInsideSupportLink>();
}

ToolsInsideStorageStats ToolsInsideQueryService::getStorageStats(QString *errorMessage) const
{
    return m_repository ? m_repository->getStorageStats(errorMessage) : ToolsInsideStorageStats();
}

} // namespace qtllm::toolsinside
