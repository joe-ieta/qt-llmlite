#include "conversationclient.h"

#include "../core/qtllmclient.h"
#include "../providers/illmprovider.h"

#include <QDateTime>
#include <QStringList>
#include <QUuid>
#include <QtGlobal>

namespace qtllm::chat {

namespace {

QString defaultSessionTitle(int index)
{
    return QStringLiteral("Session %1").arg(index);
}

} // namespace

ConversationClient::ConversationClient(QString uid, QObject *parent)
    : QObject(parent)
    , m_uid(std::move(uid))
    , m_llmClient(new QtLLMClient(this))
{
    ensureAtLeastOneSession();

    connect(m_llmClient, &QtLLMClient::tokenReceived, this, [this](const QString &token) {
        m_pendingAssistantText += token;
        emit tokenReceived(token);
    });

    connect(m_llmClient, &QtLLMClient::completed, this, [this](const QString &text) {
        const QString assistantText = m_pendingAssistantText.isEmpty() ? text : m_pendingAssistantText;
        m_pendingAssistantText.clear();
        appendMessage(QStringLiteral("assistant"), assistantText);
        emit completed(assistantText);
    });

    connect(m_llmClient, &QtLLMClient::errorOccurred, this, [this](const QString &message) {
        m_pendingAssistantText.clear();
        emit errorOccurred(message);
    });
}

QString ConversationClient::uid() const
{
    return m_uid;
}

void ConversationClient::setConfig(const LlmConfig &config)
{
    m_config = config;
    m_llmClient->setConfig(config);
    emit configChanged();
}

LlmConfig ConversationClient::config() const
{
    return m_config;
}

void ConversationClient::setProvider(std::unique_ptr<ILLMProvider> provider)
{
    m_llmClient->setProvider(std::move(provider));
}

bool ConversationClient::setProviderByName(const QString &providerName)
{
    return m_llmClient->setProviderByName(providerName);
}

void ConversationClient::setProfile(const profile::ClientProfile &profile)
{
    m_profile = profile;
    emit profileChanged();
}

profile::ClientProfile ConversationClient::profile() const
{
    return m_profile;
}

QString ConversationClient::activeSessionId() const
{
    return m_activeSessionId;
}

QString ConversationClient::createSession(const QString &title)
{
    ConversationSessionSnapshot session;
    session.sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session.title = title.trimmed().isEmpty() ? defaultSessionTitle(m_sessions.size() + 1) : title.trimmed();
    session.createdAt = QDateTime::currentDateTimeUtc();
    session.updatedAt = session.createdAt;
    m_sessions.append(session);

    m_activeSessionId = session.sessionId;

    emit sessionsChanged();
    emit activeSessionChanged(m_activeSessionId);
    emit historyChanged();

    return session.sessionId;
}

bool ConversationClient::switchSession(const QString &sessionId)
{
    const QString id = sessionId.trimmed();
    if (id.isEmpty()) {
        return false;
    }

    if (m_activeSessionId == id) {
        return true;
    }

    if (findSessionIndex(id) < 0) {
        return false;
    }

    m_activeSessionId = id;
    emit activeSessionChanged(m_activeSessionId);
    emit historyChanged();
    return true;
}

QStringList ConversationClient::sessionIds() const
{
    QStringList ids;
    ids.reserve(m_sessions.size());
    for (const ConversationSessionSnapshot &session : m_sessions) {
        ids.append(session.sessionId);
    }
    return ids;
}

QString ConversationClient::sessionTitle(const QString &sessionId) const
{
    const int index = findSessionIndex(sessionId);
    if (index < 0) {
        return QString();
    }
    return m_sessions.at(index).title;
}

void ConversationClient::setHistory(const QVector<LlmMessage> &history)
{
    ConversationSessionSnapshot *session = activeSession();
    if (!session) {
        return;
    }

    session->history = history;
    session->updatedAt = QDateTime::currentDateTimeUtc();
    emit historyChanged();
    emit sessionsChanged();
}

QVector<LlmMessage> ConversationClient::history() const
{
    const ConversationSessionSnapshot *session = activeSession();
    return session ? session->history : QVector<LlmMessage>();
}

void ConversationClient::clearHistory()
{
    ConversationSessionSnapshot *session = activeSession();
    if (!session) {
        return;
    }

    session->history.clear();
    session->updatedAt = QDateTime::currentDateTimeUtc();
    emit historyChanged();
    emit sessionsChanged();
}

void ConversationClient::sendUserMessage(const QString &content)
{
    const QString trimmed = content.trimmed();
    if (trimmed.isEmpty()) {
        return;
    }

    appendMessage(QStringLiteral("user"), trimmed);
    m_pendingAssistantText.clear();
    m_llmClient->sendPrompt(buildPromptForNextTurn(trimmed));
}

ConversationSnapshot ConversationClient::snapshot() const
{
    ConversationSnapshot state;
    state.uid = m_uid;
    state.config = m_config;
    state.profile = m_profile;
    state.activeSessionId = m_activeSessionId;
    state.sessions = m_sessions;
    return state;
}

void ConversationClient::restoreFromSnapshot(const ConversationSnapshot &snapshot)
{
    m_uid = snapshot.uid;
    setConfig(snapshot.config);
    setProfile(snapshot.profile);
    m_sessions = snapshot.sessions;
    m_activeSessionId = snapshot.activeSessionId;
    ensureAtLeastOneSession();

    emit sessionsChanged();
    emit activeSessionChanged(m_activeSessionId);
    emit historyChanged();
}

QString ConversationClient::buildPromptForNextTurn(const QString &currentUserMessage) const
{
    QStringList lines;

    if (!m_profile.systemPrompt.trimmed().isEmpty()) {
        lines.append(QStringLiteral("[system]\n") + m_profile.systemPrompt.trimmed());
    }
    if (!m_profile.persona.trimmed().isEmpty()) {
        lines.append(QStringLiteral("[persona]\n") + m_profile.persona.trimmed());
    }
    if (!m_profile.thinkingStyle.trimmed().isEmpty()) {
        lines.append(QStringLiteral("[thinking-style]\n") + m_profile.thinkingStyle.trimmed());
    }

    const QVector<LlmMessage> messages = history();
    const int maxMessages = qMax(1, m_profile.memoryPolicy.maxHistoryMessages);
    const int startIndex = qMax(0, messages.size() - maxMessages);
    if (!messages.isEmpty()) {
        lines.append(QStringLiteral("[history]"));
        for (int i = startIndex; i < messages.size(); ++i) {
            const LlmMessage &message = messages.at(i);
            lines.append(QStringLiteral("%1: %2").arg(message.role, message.content));
        }
    }

    lines.append(QStringLiteral("[current-user]\n") + currentUserMessage);
    return lines.join(QStringLiteral("\n\n"));
}

void ConversationClient::appendMessage(const QString &role, const QString &content)
{
    ConversationSessionSnapshot *session = activeSession();
    if (!session) {
        return;
    }

    LlmMessage message;
    message.role = role;
    message.content = content;
    session->history.append(message);
    session->updatedAt = QDateTime::currentDateTimeUtc();

    emit historyChanged();
    emit sessionsChanged();
}

int ConversationClient::findSessionIndex(const QString &sessionId) const
{
    for (int i = 0; i < m_sessions.size(); ++i) {
        if (m_sessions.at(i).sessionId == sessionId) {
            return i;
        }
    }
    return -1;
}

ConversationSessionSnapshot *ConversationClient::activeSession()
{
    const int index = findSessionIndex(m_activeSessionId);
    if (index < 0) {
        return nullptr;
    }
    return &m_sessions[index];
}

const ConversationSessionSnapshot *ConversationClient::activeSession() const
{
    const int index = findSessionIndex(m_activeSessionId);
    if (index < 0) {
        return nullptr;
    }
    return &m_sessions.at(index);
}

void ConversationClient::ensureAtLeastOneSession()
{
    if (!m_sessions.isEmpty()) {
        if (findSessionIndex(m_activeSessionId) < 0) {
            m_activeSessionId = m_sessions.first().sessionId;
        }
        return;
    }

    ConversationSessionSnapshot session;
    session.sessionId = QStringLiteral("session-1");
    session.title = defaultSessionTitle(1);
    session.createdAt = QDateTime::currentDateTimeUtc();
    session.updatedAt = session.createdAt;
    m_sessions.append(session);
    m_activeSessionId = session.sessionId;
}

} // namespace qtllm::chat
