#pragma once

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

namespace qtllm::toolsinside {
class ToolsInsideQueryService;
class ToolsInsideAdminService;
}

class ToolsInsideBrowser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString workspaceRoot READ workspaceRoot WRITE setWorkspaceRoot NOTIFY workspaceRootChanged)
    Q_PROPERTY(QStringList workspaceHistory READ workspaceHistory NOTIFY workspaceHistoryChanged)
    Q_PROPERTY(QStringList workspaceFavorites READ workspaceFavorites NOTIFY workspaceFavoritesChanged)
    Q_PROPERTY(QVariantList clients READ clients NOTIFY clientsChanged)
    Q_PROPERTY(QVariantList sessions READ sessions NOTIFY sessionsChanged)
    Q_PROPERTY(QVariantList traces READ traces NOTIFY tracesChanged)
    Q_PROPERTY(QVariantList timeline READ timeline NOTIFY timelineChanged)
    Q_PROPERTY(QVariantList timelineLanes READ timelineLanes NOTIFY timelineLanesChanged)
    Q_PROPERTY(qint64 timelineDurationMs READ timelineDurationMs NOTIFY timelineDurationChanged)
    Q_PROPERTY(int timelineTickMs READ timelineTickMs CONSTANT)
    Q_PROPERTY(QVariantList toolCalls READ toolCalls NOTIFY toolCallsChanged)
    Q_PROPERTY(QVariantList supportLinks READ supportLinks NOTIFY supportLinksChanged)
    Q_PROPERTY(QVariantList artifacts READ artifacts NOTIFY artifactsChanged)
    Q_PROPERTY(QString selectedClientId READ selectedClientId NOTIFY selectedClientIdChanged)
    Q_PROPERTY(QString selectedSessionId READ selectedSessionId NOTIFY selectedSessionIdChanged)
    Q_PROPERTY(QString selectedTraceId READ selectedTraceId NOTIFY selectedTraceIdChanged)
    Q_PROPERTY(QVariantMap traceSummary READ traceSummary NOTIFY traceSummaryChanged)
    Q_PROPERTY(QVariantMap inspector READ inspector NOTIFY inspectorChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)

public:
    explicit ToolsInsideBrowser(QObject *parent = nullptr);

    QString workspaceRoot() const;
    QStringList workspaceHistory() const;
    QStringList workspaceFavorites() const;
    void setWorkspaceRoot(const QString &workspaceRoot);

    QVariantList clients() const;
    QVariantList sessions() const;
    QVariantList traces() const;
    QVariantList timeline() const;
    QVariantList timelineLanes() const;
    qint64 timelineDurationMs() const;
    int timelineTickMs() const;
    QVariantList toolCalls() const;
    QVariantList supportLinks() const;
    QVariantList artifacts() const;
    QString selectedClientId() const;
    QString selectedSessionId() const;
    QString selectedTraceId() const;
    QVariantMap traceSummary() const;
    QVariantMap inspector() const;
    QString statusText() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void chooseWorkspaceRoot();
    Q_INVOKABLE void selectWorkspaceHistory(const QString &workspaceRoot);
    Q_INVOKABLE void toggleWorkspaceFavorite(const QString &workspaceRoot);
    Q_INVOKABLE void removeWorkspaceFavorite(const QString &workspaceRoot);
    Q_INVOKABLE void clearWorkspaceHistory();
    Q_INVOKABLE bool isFavoriteWorkspace(const QString &workspaceRoot) const;
    Q_INVOKABLE void selectClient(const QString &clientId);
    Q_INVOKABLE void selectSession(const QString &sessionId);
    Q_INVOKABLE void selectTrace(const QString &traceId);
    Q_INVOKABLE bool archiveSelectedTrace();
    Q_INVOKABLE bool purgeSelectedTrace();
    Q_INVOKABLE void inspectTimelineEntry(const QVariantMap &entry);
    Q_INVOKABLE void inspectToolCall(const QVariantMap &toolCall);
    Q_INVOKABLE void inspectSupportLink(const QVariantMap &supportLink);
    Q_INVOKABLE void inspectArtifact(const QVariantMap &artifact);
    Q_INVOKABLE void clearInspector();

signals:
    void workspaceRootChanged();
    void workspaceHistoryChanged();
    void workspaceFavoritesChanged();
    void clientsChanged();
    void sessionsChanged();
    void tracesChanged();
    void timelineChanged();
    void timelineLanesChanged();
    void timelineDurationChanged();
    void toolCallsChanged();
    void supportLinksChanged();
    void artifactsChanged();
    void selectedClientIdChanged();
    void selectedSessionIdChanged();
    void selectedTraceIdChanged();
    void traceSummaryChanged();
    void inspectorChanged();
    void statusTextChanged();

private:
    void reloadClients();
    void reloadSessions();
    void reloadTraces();
    void reloadTraceDetail();
    void resetTraceDetail();
    void ensureRuntimeConfigured();
    void loadWorkspacePreferences();
    void persistWorkspacePreferences() const;
    void rememberWorkspaceRoot(const QString &workspaceRoot);
    QString normalizeWorkspaceRoot(const QString &workspaceRoot) const;
    void setStatusText(const QString &statusText);
    void setInspector(const QVariantMap &inspector);
    QVariantMap findArtifactById(const QString &artifactId) const;
    QVariantMap findToolCallById(const QString &toolCallId) const;
    QString loadArtifactContent(const QVariantMap &artifact) const;

private:
    static constexpr int kTimelineTickMs = 6;
    static constexpr int kMaxWorkspaceHistory = 12;
    static constexpr int kMaxWorkspaceFavorites = 10;

    QString m_workspaceRoot;
    QStringList m_workspaceHistory;
    QStringList m_workspaceFavorites;
    QVariantList m_clients;
    QVariantList m_sessions;
    QVariantList m_traces;
    QVariantList m_timeline;
    QVariantList m_timelineLanes;
    qint64 m_timelineDurationMs = kTimelineTickMs;
    QVariantList m_toolCalls;
    QVariantList m_supportLinks;
    QVariantList m_artifacts;
    QString m_selectedClientId;
    QString m_selectedSessionId;
    QString m_selectedTraceId;
    QVariantMap m_traceSummary;
    QVariantMap m_inspector;
    QString m_statusText;
};
