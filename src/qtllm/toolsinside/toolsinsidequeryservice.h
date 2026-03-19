#pragma once

#include "toolsinsiderepository.h"

#include <memory>
#include <optional>

namespace qtllm::toolsinside {

class ToolsInsideQueryService
{
public:
    explicit ToolsInsideQueryService(std::shared_ptr<ToolsInsideRepository> repository = nullptr);

    void setRepository(const std::shared_ptr<ToolsInsideRepository> &repository);

    QList<ToolsInsideClientSummary> listClients(QString *errorMessage = nullptr) const;
    QList<ToolsInsideSessionSummary> listSessions(const QString &clientId,
                                                  QString *errorMessage = nullptr) const;
    QList<ToolsInsideTraceSummary> listTraces(const ToolsInsideTraceFilter &filter,
                                              QString *errorMessage = nullptr) const;
    std::optional<ToolsInsideTraceSummary> getTraceSummary(const QString &traceId,
                                                           QString *errorMessage = nullptr) const;
    QList<ToolsInsideEventRecord> getTraceTimeline(const QString &traceId,
                                                   QString *errorMessage = nullptr) const;
    QList<ToolsInsideSpanRecord> getTraceSpans(const QString &traceId,
                                               QString *errorMessage = nullptr) const;
    QList<ToolsInsideArtifactRef> getTraceArtifacts(const QString &traceId,
                                                    QString *errorMessage = nullptr) const;
    QList<ToolsInsideToolCallRecord> getTraceToolCalls(const QString &traceId,
                                                       QString *errorMessage = nullptr) const;
    QList<ToolsInsideSupportLink> getTraceSupportLinks(const QString &traceId,
                                                       QString *errorMessage = nullptr) const;
    ToolsInsideStorageStats getStorageStats(QString *errorMessage = nullptr) const;

private:
    std::shared_ptr<ToolsInsideRepository> m_repository;
};

} // namespace qtllm::toolsinside
