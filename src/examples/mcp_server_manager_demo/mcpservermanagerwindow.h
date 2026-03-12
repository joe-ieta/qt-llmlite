#pragma once

#include "../../qtllm/tools/mcp/defaultmcpclient.h"
#include "../../qtllm/tools/mcp/mcpservermanager.h"

#include <QHash>
#include <QWidget>

#include <memory>

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QComboBox;
class QTextEdit;
class QPushButton;
class QLabel;

class McpServerManagerWindow : public QWidget
{
    Q_OBJECT
public:
    explicit McpServerManagerWindow(QWidget *parent = nullptr);

private slots:
    void onScanClicked();
    void onRegisterSelectedClicked();
    void onAddManualClicked();
    void onRemoveRegisteredClicked();
    void onCandidateSelectionChanged();
    void onRegisteredSelectionChanged();
    void onRefreshDetailsClicked();

private:
    void loadRegisteredServers();
    void renderCandidateList();
    void renderRegisteredList();

    void setSelectedServer(const qtllm::tools::mcp::McpServerDefinition &server);
    void clearDetails();
    void refreshDetails(const qtllm::tools::mcp::McpServerDefinition &server);

    void appendLog(const QString &line);

    QVector<qtllm::tools::mcp::McpServerDefinition> scanPossibleServers() const;
    static QVector<qtllm::tools::mcp::McpServerDefinition> parseServersFromJson(const QJsonObject &root);

    static bool maybeAvailable(const qtllm::tools::mcp::McpServerDefinition &server);
    static qtllm::tools::mcp::McpServerDefinition buildManualServer(const QString &serverId,
                                                                    const QString &name,
                                                                    const QString &transport,
                                                                    const QString &command,
                                                                    const QString &args,
                                                                    const QString &url,
                                                                    const QString &envJson,
                                                                    const QString &headersJson,
                                                                    int timeoutMs,
                                                                    QString *errorMessage);

private:
    std::shared_ptr<qtllm::tools::mcp::McpServerManager> m_serverManager;
    std::shared_ptr<qtllm::tools::mcp::IMcpClient> m_mcpClient;

    QHash<QString, qtllm::tools::mcp::McpServerDefinition> m_candidateById;
    QHash<QString, qtllm::tools::mcp::McpServerDefinition> m_registeredById;

    QListWidget *m_candidateList;
    QPushButton *m_scanButton;
    QPushButton *m_registerButton;

    QLineEdit *m_idEdit;
    QLineEdit *m_nameEdit;
    QComboBox *m_transportCombo;
    QLineEdit *m_commandEdit;
    QLineEdit *m_argsEdit;
    QLineEdit *m_urlEdit;
    QTextEdit *m_envEdit;
    QTextEdit *m_headersEdit;
    QLineEdit *m_timeoutEdit;
    QPushButton *m_addManualButton;

    QListWidget *m_registeredList;
    QPushButton *m_removeButton;

    QLabel *m_detailTitle;
    QTextEdit *m_detailMeta;
    QTextEdit *m_toolsView;
    QTextEdit *m_resourcesView;
    QTextEdit *m_promptsView;
    QPushButton *m_refreshDetailsButton;

    QTextEdit *m_logView;

    qtllm::tools::mcp::McpServerDefinition m_selectedServer;
    bool m_hasSelectedServer = false;
};
