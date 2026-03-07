#pragma once

#include "../../qtllm/chat/conversationclientfactory.h"

#include <QMetaObject>
#include <QPointer>
#include <QSharedPointer>
#include <QWidget>

#include <memory>

namespace qtllm::chat {
class ConversationClient;
}

namespace qtllm::storage {
class ConversationRepository;
}

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QTextEdit;
class QPushButton;
class QComboBox;
class QNetworkAccessManager;
class QNetworkReply;

class MultiClientWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MultiClientWindow(QWidget *parent = nullptr);

private slots:
    void onNewClientClicked();
    void onClientSelectionChanged();
    void onNewSessionClicked();
    void onSessionSelectionChanged();
    void onSendClicked();
    void onApplyProfileClicked();
    void onApplyConfigClicked();
    void onRefreshModelsClicked();
    void onModelsReplyFinished();

private:
    QSharedPointer<qtllm::chat::ConversationClient> activeClient() const;
    void loadClients();
    void addClientToList(const QString &clientId);
    void bindActiveClient(const QSharedPointer<qtllm::chat::ConversationClient> &client);
    void rebuildSessionList(const QSharedPointer<qtllm::chat::ConversationClient> &client);
    void renderActiveHistory(const QSharedPointer<qtllm::chat::ConversationClient> &client);
    void refreshEditorFromActiveClient(const QSharedPointer<qtllm::chat::ConversationClient> &client);
    bool applyConfigToActiveClient(bool showMessage);
    void refreshModels();

private:
    std::shared_ptr<qtllm::storage::ConversationRepository> m_repository;
    std::unique_ptr<qtllm::chat::ConversationClientFactory> m_factory;

    QListWidget *m_clientList;
    QPushButton *m_newClientButton;

    QListWidget *m_sessionList;
    QPushButton *m_newSessionButton;

    QTextEdit *m_output;
    QLineEdit *m_input;
    QPushButton *m_sendButton;

    QComboBox *m_providerCombo;
    QLineEdit *m_baseUrlEdit;
    QLineEdit *m_apiKeyEdit;
    QComboBox *m_modelCombo;
    QPushButton *m_reloadModelsButton;
    QPushButton *m_applyConfigButton;
    QNetworkAccessManager *m_networkManager;
    QPointer<QNetworkReply> m_modelsReply;

    QTextEdit *m_systemPromptEdit;
    QLineEdit *m_personaEdit;
    QLineEdit *m_thinkingStyleEdit;
    QLineEdit *m_skillsEdit;
    QLineEdit *m_capabilitiesEdit;
    QLineEdit *m_preferencesEdit;
    QLineEdit *m_historyWindowEdit;
    QPushButton *m_applyProfileButton;

    QMetaObject::Connection m_tokenConn;
    QMetaObject::Connection m_completedConn;
    QMetaObject::Connection m_errorConn;
    QMetaObject::Connection m_historyConn;
    QMetaObject::Connection m_sessionsConn;
    QMetaObject::Connection m_activeSessionConn;
};
