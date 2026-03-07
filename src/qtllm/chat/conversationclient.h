#pragma once

#include "conversationsnapshot.h"

#include <QObject>
#include <QVector>
#include <memory>

namespace qtllm {

class ILLMProvider;
class QtLLMClient;

namespace chat {

class ConversationClient : public QObject
{
    Q_OBJECT
public:
    explicit ConversationClient(QString uid, QObject *parent = nullptr);

    QString uid() const;

    void setConfig(const LlmConfig &config);
    LlmConfig config() const;

    void setProvider(std::unique_ptr<ILLMProvider> provider);
    bool setProviderByName(const QString &providerName);

    void setProfile(const profile::ClientProfile &profile);
    profile::ClientProfile profile() const;

    QString activeSessionId() const;
    QString createSession(const QString &title = QString());
    bool switchSession(const QString &sessionId);
    QStringList sessionIds() const;
    QString sessionTitle(const QString &sessionId) const;

    void setHistory(const QVector<LlmMessage> &history);
    QVector<LlmMessage> history() const;
    void clearHistory();

    void sendUserMessage(const QString &content);

    ConversationSnapshot snapshot() const;
    void restoreFromSnapshot(const ConversationSnapshot &snapshot);

signals:
    void tokenReceived(const QString &token);
    void completed(const QString &text);
    void errorOccurred(const QString &message);
    void historyChanged();
    void sessionsChanged();
    void activeSessionChanged(const QString &sessionId);
    void configChanged();
    void profileChanged();

private:
    QString buildPromptForNextTurn(const QString &currentUserMessage) const;
    void appendMessage(const QString &role, const QString &content);
    int findSessionIndex(const QString &sessionId) const;
    ConversationSessionSnapshot *activeSession();
    const ConversationSessionSnapshot *activeSession() const;
    void ensureAtLeastOneSession();

private:
    QString m_uid;
    LlmConfig m_config;
    profile::ClientProfile m_profile;
    QVector<ConversationSessionSnapshot> m_sessions;
    QString m_activeSessionId;
    QString m_pendingAssistantText;
    QtLLMClient *m_llmClient;
};

} // namespace chat
} // namespace qtllm
