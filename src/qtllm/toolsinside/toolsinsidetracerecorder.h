#pragma once

#include "toolsinsideartifactstore.h"
#include "toolsinsiderepository.h"
#include "toolsinside_types.h"

#include "../core/llmtypes.h"
#include "../tools/runtime/toolruntime_types.h"

#include <QHash>
#include <QMutex>

#include <memory>

namespace qtllm::toolsinside {

class ToolsInsideTraceRecorder
{
public:
    ToolsInsideTraceRecorder(std::shared_ptr<ToolsInsideRepository> repository,
                             std::shared_ptr<ToolsInsideArtifactStore> artifactStore,
                             const ToolsInsideStoragePolicy &storagePolicy,
                             std::shared_ptr<IToolsInsideRedactionPolicy> redactionPolicy);

    QString startTrace(const QString &clientId,
                       const QString &sessionId,
                       const QString &traceId,
                       const QString &turnInput,
                       const QString &provider,
                       const QString &model,
                       const QString &vendor);

    void recordToolSelection(const QString &traceId,
                             const QStringList &toolIds,
                             const QString &schemaText);

    void recordRequestPrepared(const QString &clientId,
                               const QString &sessionId,
                               const QString &traceId,
                               const QString &requestJson);

    void recordRequestDispatched(const QString &clientId,
                                 const QString &sessionId,
                                 const QString &traceId,
                                 const QString &requestId,
                                 const QString &provider,
                                 const QString &model,
                                 const QString &url,
                                 const QString &payloadJson,
                                 int messageCount,
                                 int toolCount);

    void recordFirstStreamToken(const QString &traceId,
                                const QString &requestId,
                                const QString &channel);

    void recordResponseParsed(const QString &traceId,
                              const QString &requestId,
                              const qtllm::LlmResponse &response,
                              const QString &assistantText);

    void recordToolCallsParsed(const qtllm::tools::runtime::ToolExecutionContext &context,
                               const QString &requestId,
                               const QString &adapterId,
                               int roundIndex,
                               const QList<qtllm::tools::runtime::ToolCallRequest> &requests);

    void recordToolBatchStarted(const qtllm::tools::runtime::ToolExecutionContext &context,
                                const QString &requestId,
                                int roundIndex,
                                int requestCount);

    void recordToolCallStarted(const qtllm::tools::runtime::ToolExecutionContext &context,
                               const QString &requestId,
                               int roundIndex,
                               const qtllm::tools::runtime::ToolCallRequest &request);

    void recordToolCallFinished(const qtllm::tools::runtime::ToolExecutionContext &context,
                                const QString &requestId,
                                int roundIndex,
                                const qtllm::tools::runtime::ToolCallRequest &request,
                                const qtllm::tools::runtime::ToolExecutionResult &result);

    void recordFollowUpPrompt(const qtllm::tools::runtime::ToolExecutionContext &context,
                              const QString &requestId,
                              int roundIndex,
                              const QString &prompt,
                              const QList<qtllm::tools::runtime::ToolExecutionResult> &results);

    void recordFailureGuard(const qtllm::tools::runtime::ToolExecutionContext &context,
                            const QString &requestId,
                            int roundIndex,
                            int consecutiveFailures,
                            const QString &reason);

    void recordTraceCompleted(const QString &clientId,
                              const QString &sessionId,
                              const QString &traceId,
                              const QString &requestId,
                              const QString &finalText,
                              const QString &finishReason);

    void recordTraceError(const QString &clientId,
                          const QString &sessionId,
                          const QString &traceId,
                          const QString &requestId,
                          const QString &message,
                          const QString &category);

private:
    QString ensureSpanId(const QString &key) const;
    QString requestSpanKey(const QString &traceId, const QString &requestId) const;
    QString toolSpanKey(const QString &traceId, const QString &requestId, const QString &toolCallId) const;
    QString batchSpanKey(const QString &traceId, const QString &requestId, int roundIndex) const;
    QString persistArtifact(const QString &clientId,
                            const QString &sessionId,
                            const QString &traceId,
                            const QString &kind,
                            const QString &mimeType,
                            const QByteArray &payload,
                            const QJsonObject &metadata = QJsonObject(),
                            const QString &preferredExtension = QString()) const;

private:
    std::shared_ptr<ToolsInsideRepository> m_repository;
    std::shared_ptr<ToolsInsideArtifactStore> m_artifactStore;
    ToolsInsideStoragePolicy m_storagePolicy;
    std::shared_ptr<IToolsInsideRedactionPolicy> m_redactionPolicy;
    mutable QMutex m_mutex;
    mutable QHash<QString, QString> m_spanIdByKey;
    mutable QHash<QString, bool> m_firstContentTokenSeen;
    mutable QHash<QString, bool> m_firstReasoningTokenSeen;
    mutable QHash<QString, QString> m_clientIdByTrace;
    mutable QHash<QString, QString> m_sessionIdByTrace;
};

} // namespace qtllm::toolsinside
