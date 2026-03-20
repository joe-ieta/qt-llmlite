#include "mcpservermanagerwindow.h"

#include "mcpchatwindow.h"

#include "../../qtllm/logging/logtypes.h"
#include "../../qtllm/logging/qtllmlogger.h"
#include "../../qtllm/logging/signallogsink.h"
#include "../../qtllm/tools/mcp/mcpserverregistry.h"

#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSplitter>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

QString normalizeId(const QString &value)
{
    return value.trimmed().toLower();
}

QString formatServerSummary(const qtllm::tools::mcp::McpServerDefinition &server)
{
    return QStringLiteral("%1 (%2, %3)")
        .arg(server.serverId,
             server.transport,
             server.enabled ? QStringLiteral("enabled") : QStringLiteral("disabled"));
}

QJsonObject parseJsonObjectText(const QString &text, QString *errorMessage)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return QJsonObject();
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(trimmed.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("JSON parse error: ") + parseError.errorString();
        }
        return QJsonObject();
    }

    return doc.object();
}

QString formatLogEvent(const qtllm::logging::LogEvent &event)
{
    QString line = QStringLiteral("[%1][%2][%3]")
                       .arg(event.timestampUtc.toString(QStringLiteral("HH:mm:ss.zzz")),
                            qtllm::logging::logLevelToString(event.level),
                            event.category);
    line += QStringLiteral(" ") + event.message;
    if (!event.fields.isEmpty()) {
        line += QStringLiteral(" ") + QString::fromUtf8(QJsonDocument(event.fields).toJson(QJsonDocument::Compact));
    }
    return line;
}

QStringList parseArgs(const QString &text)
{
    QStringList items = text.split(',', Qt::SkipEmptyParts);
    for (QString &item : items) {
        item = item.trimmed();
    }
    items.removeAll(QString());
    return items;
}

} // namespace

McpServerManagerWindow::McpServerManagerWindow(QWidget *parent)
    : QWidget(parent)
    , m_serverManager(std::make_shared<qtllm::tools::mcp::McpServerManager>())
    , m_mcpClient(std::make_shared<qtllm::tools::mcp::DefaultMcpClient>())
    , m_logSink(std::make_shared<qtllm::logging::SignalLogSink>())
    , m_candidateList(new QListWidget(this))
    , m_scanButton(new QPushButton(QStringLiteral("Scan"), this))
    , m_registerButton(new QPushButton(QStringLiteral("Register Selected"), this))
    , m_idEdit(new QLineEdit(this))
    , m_nameEdit(new QLineEdit(this))
    , m_transportCombo(new QComboBox(this))
    , m_commandEdit(new QLineEdit(this))
    , m_argsEdit(new QLineEdit(this))
    , m_urlEdit(new QLineEdit(this))
    , m_envEdit(new QTextEdit(this))
    , m_headersEdit(new QTextEdit(this))
    , m_timeoutEdit(new QLineEdit(this))
    , m_addManualButton(new QPushButton(QStringLiteral("Add"), this))
    , m_registeredList(new QListWidget(this))
    , m_removeButton(new QPushButton(QStringLiteral("Remove Selected"), this))
    , m_openChatButton(new QPushButton(QStringLiteral("Open Chat Window"), this))
    , m_detailTitle(new QLabel(QStringLiteral("No server selected"), this))
    , m_detailMeta(new QTextEdit(this))
    , m_toolsView(new QTextEdit(this))
    , m_resourcesView(new QTextEdit(this))
    , m_promptsView(new QTextEdit(this))
    , m_refreshDetailsButton(new QPushButton(QStringLiteral("Refresh Details"), this))
    , m_logView(new QTextEdit(this))
{
    setWindowTitle(QStringLiteral("MCP Server Manager Demo"));
    resize(1400, 860);

    qtllm::logging::QtLlmLogger::instance().addSink(m_logSink);
    connect(m_logSink.get(), &qtllm::logging::SignalLogSink::logEventReceived,
            this, &McpServerManagerWindow::onLogEventReceived);

    m_transportCombo->addItem(QStringLiteral("stdio"));
    m_transportCombo->addItem(QStringLiteral("streamable-http"));
    m_transportCombo->addItem(QStringLiteral("sse"));

    m_envEdit->setPlaceholderText(QStringLiteral("{}"));
    m_headersEdit->setPlaceholderText(QStringLiteral("{}"));
    m_timeoutEdit->setText(QStringLiteral("30000"));
    m_argsEdit->setPlaceholderText(QStringLiteral("arg1,arg2,arg3"));

    m_detailMeta->setReadOnly(true);
    m_toolsView->setReadOnly(true);
    m_resourcesView->setReadOnly(true);
    m_promptsView->setReadOnly(true);
    m_logView->setReadOnly(true);

    auto *discoverLayout = new QVBoxLayout();
    discoverLayout->addWidget(new QLabel(QStringLiteral("Scan Available MCP Servers"), this));
    discoverLayout->addWidget(m_candidateList, 1);
    auto *discoverButtons = new QHBoxLayout();
    discoverButtons->addWidget(m_scanButton);
    discoverButtons->addWidget(m_registerButton);
    discoverLayout->addLayout(discoverButtons);

    auto *manualForm = new QFormLayout();
    manualForm->addRow(QStringLiteral("Server ID"), m_idEdit);
    manualForm->addRow(QStringLiteral("Name"), m_nameEdit);
    manualForm->addRow(QStringLiteral("Transport"), m_transportCombo);
    manualForm->addRow(QStringLiteral("Command (stdio)"), m_commandEdit);
    manualForm->addRow(QStringLiteral("Args CSV"), m_argsEdit);
    manualForm->addRow(QStringLiteral("URL (http/sse)"), m_urlEdit);
    manualForm->addRow(QStringLiteral("Env JSON"), m_envEdit);
    manualForm->addRow(QStringLiteral("Headers JSON"), m_headersEdit);
    manualForm->addRow(QStringLiteral("Timeout ms"), m_timeoutEdit);

    auto *manualLayout = new QVBoxLayout();
    manualLayout->addWidget(new QLabel(QStringLiteral("Add MCP Server Manually"), this));
    manualLayout->addLayout(manualForm);
    manualLayout->addWidget(m_addManualButton);

    auto *leftPaneLayout = new QVBoxLayout();
    leftPaneLayout->addLayout(discoverLayout, 3);
    leftPaneLayout->addLayout(manualLayout, 2);
    auto *leftPane = new QWidget(this);
    leftPane->setLayout(leftPaneLayout);

    auto *registeredLayout = new QVBoxLayout();
    registeredLayout->addWidget(new QLabel(QStringLiteral("Registered MCP Servers"), this));
    registeredLayout->addWidget(m_registeredList, 1);
    registeredLayout->addWidget(m_removeButton);
    registeredLayout->addWidget(m_openChatButton);
    auto *registeredPane = new QWidget(this);
    registeredPane->setLayout(registeredLayout);

    auto *detailTabs = new QTabWidget(this);
    detailTabs->addTab(m_toolsView, QStringLiteral("Tools"));
    detailTabs->addTab(m_resourcesView, QStringLiteral("Resources"));
    detailTabs->addTab(m_promptsView, QStringLiteral("Prompts"));

    auto *detailLayout = new QVBoxLayout();
    detailLayout->addWidget(m_detailTitle);
    detailLayout->addWidget(m_detailMeta, 1);
    detailLayout->addWidget(m_refreshDetailsButton);
    detailLayout->addWidget(detailTabs, 2);
    detailLayout->addWidget(new QLabel(QStringLiteral("Logs"), this));
    detailLayout->addWidget(m_logView, 1);
    auto *detailPane = new QWidget(this);
    detailPane->setLayout(detailLayout);

    auto *splitter = new QSplitter(this);
    splitter->addWidget(leftPane);
    splitter->addWidget(registeredPane);
    splitter->addWidget(detailPane);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 2);

    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->addWidget(splitter);
    setLayout(rootLayout);

    connect(m_scanButton, &QPushButton::clicked, this, &McpServerManagerWindow::onScanClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &McpServerManagerWindow::onRegisterSelectedClicked);
    connect(m_addManualButton, &QPushButton::clicked, this, &McpServerManagerWindow::onAddManualClicked);
    connect(m_removeButton, &QPushButton::clicked, this, &McpServerManagerWindow::onRemoveRegisteredClicked);
    connect(m_candidateList, &QListWidget::itemSelectionChanged, this, &McpServerManagerWindow::onCandidateSelectionChanged);
    connect(m_registeredList, &QListWidget::itemSelectionChanged, this, &McpServerManagerWindow::onRegisteredSelectionChanged);
    connect(m_refreshDetailsButton, &QPushButton::clicked, this, &McpServerManagerWindow::onRefreshDetailsClicked);
    connect(m_openChatButton, &QPushButton::clicked, this, &McpServerManagerWindow::onOpenChatClicked);

    loadRegisteredServers();
    onScanClicked();
}

McpServerManagerWindow::~McpServerManagerWindow()
{
    qtllm::logging::QtLlmLogger::instance().removeSink(m_logSink);
}

void McpServerManagerWindow::onScanClicked()
{
    const QVector<qtllm::tools::mcp::McpServerDefinition> scanned = scanPossibleServers();
    m_candidateById.clear();

    for (const qtllm::tools::mcp::McpServerDefinition &server : scanned) {
        const QString id = normalizeId(server.serverId);
        if (id.isEmpty() || m_registeredById.contains(id)) {
            continue;
        }
        m_candidateById.insert(id, server);
    }

    renderCandidateList();
    appendLog(QStringLiteral("Scan finished. Candidates: %1").arg(m_candidateById.size()));
}

void McpServerManagerWindow::onRegisterSelectedClicked()
{
    QListWidgetItem *item = m_candidateList->currentItem();
    if (!item) {
        appendLog(QStringLiteral("No candidate selected."));
        return;
    }

    const QString id = item->data(Qt::UserRole).toString();
    const auto it = m_candidateById.constFind(id);
    if (it == m_candidateById.constEnd()) {
        appendLog(QStringLiteral("Selected candidate no longer exists."));
        return;
    }

    QString errorMessage;
    if (!m_serverManager->registerServer(*it, &errorMessage)) {
        appendLog(QStringLiteral("Register failed: ") + errorMessage);
        return;
    }

    appendLog(QStringLiteral("Registered MCP server: ") + id);
    loadRegisteredServers();
    onScanClicked();
    if (m_chatWindow) {
        m_chatWindow->reloadRegisteredServers();
    }
}

void McpServerManagerWindow::onAddManualClicked()
{
    bool ok = false;
    const int timeoutMs = m_timeoutEdit->text().trimmed().toInt(&ok);
    if (!ok || timeoutMs <= 0) {
        appendLog(QStringLiteral("Invalid timeout ms"));
        return;
    }

    QString errorMessage;
    const qtllm::tools::mcp::McpServerDefinition server = buildManualServer(
        m_idEdit->text().trimmed(),
        m_nameEdit->text().trimmed(),
        m_transportCombo->currentText().trimmed(),
        m_commandEdit->text().trimmed(),
        m_argsEdit->text().trimmed(),
        m_urlEdit->text().trimmed(),
        m_envEdit->toPlainText(),
        m_headersEdit->toPlainText(),
        timeoutMs,
        &errorMessage);

    if (!errorMessage.isEmpty()) {
        appendLog(QStringLiteral("Invalid manual server: ") + errorMessage);
        return;
    }

    if (!m_serverManager->registerServer(server, &errorMessage)) {
        appendLog(QStringLiteral("Add failed: ") + errorMessage);
        return;
    }

    appendLog(QStringLiteral("Manually added MCP server: ") + server.serverId);
    loadRegisteredServers();
    onScanClicked();
    if (m_chatWindow) {
        m_chatWindow->reloadRegisteredServers();
    }

    m_registeredList->clearSelection();
    for (int i = 0; i < m_registeredList->count(); ++i) {
        QListWidgetItem *item = m_registeredList->item(i);
        if (item->data(Qt::UserRole).toString() == normalizeId(server.serverId)) {
            m_registeredList->setCurrentItem(item);
            break;
        }
    }
}

void McpServerManagerWindow::onRemoveRegisteredClicked()
{
    QListWidgetItem *item = m_registeredList->currentItem();
    if (!item) {
        appendLog(QStringLiteral("No registered server selected."));
        return;
    }

    const QString id = item->data(Qt::UserRole).toString();
    QString errorMessage;
    if (!m_serverManager->removeServer(id, &errorMessage)) {
        appendLog(QStringLiteral("Remove failed: ") + errorMessage);
        return;
    }

    appendLog(QStringLiteral("Removed MCP server: ") + id);
    loadRegisteredServers();
    onScanClicked();
    if (m_chatWindow) {
        m_chatWindow->reloadRegisteredServers();
    }

    if (m_hasSelectedServer && normalizeId(m_selectedServer.serverId) == id) {
        clearDetails();
    }
}

void McpServerManagerWindow::onCandidateSelectionChanged()
{
    if (!m_candidateList->currentItem()) {
        return;
    }

    m_registeredList->clearSelection();

    const QString id = m_candidateList->currentItem()->data(Qt::UserRole).toString();
    const auto it = m_candidateById.constFind(id);
    if (it == m_candidateById.constEnd()) {
        return;
    }

    setSelectedServer(*it);
}

void McpServerManagerWindow::onRegisteredSelectionChanged()
{
    if (!m_registeredList->currentItem()) {
        return;
    }

    m_candidateList->clearSelection();

    const QString id = m_registeredList->currentItem()->data(Qt::UserRole).toString();
    const auto it = m_registeredById.constFind(id);
    if (it == m_registeredById.constEnd()) {
        return;
    }

    setSelectedServer(*it);
}

void McpServerManagerWindow::onRefreshDetailsClicked()
{
    if (!m_hasSelectedServer) {
        appendLog(QStringLiteral("No server selected for details refresh."));
        return;
    }

    refreshDetails(m_selectedServer);
}

void McpServerManagerWindow::onOpenChatClicked()
{
    if (!m_chatWindow) {
        m_chatWindow = new McpChatWindow(m_serverManager, m_mcpClient, nullptr);
    }

    m_chatWindow->show();
    m_chatWindow->raise();
    m_chatWindow->activateWindow();
    m_chatWindow->reloadRegisteredServers();
}

void McpServerManagerWindow::loadRegisteredServers()
{
    QString errorMessage;
    if (!m_serverManager->load(&errorMessage)) {
        appendLog(QStringLiteral("Load registered servers failed: ") + errorMessage);
    }

    m_registeredById.clear();
    const QVector<qtllm::tools::mcp::McpServerDefinition> servers = m_serverManager->allServers();
    for (const qtllm::tools::mcp::McpServerDefinition &server : servers) {
        const QString id = normalizeId(server.serverId);
        if (!id.isEmpty()) {
            m_registeredById.insert(id, server);
        }
    }

    renderRegisteredList();
}

void McpServerManagerWindow::renderCandidateList()
{
    m_candidateList->clear();

    for (auto it = m_candidateById.constBegin(); it != m_candidateById.constEnd(); ++it) {
        const qtllm::tools::mcp::McpServerDefinition &server = it.value();
        const QString state = maybeAvailable(server) ? QStringLiteral("available") : QStringLiteral("unknown");
        auto *item = new QListWidgetItem(QStringLiteral("[%1] %2").arg(state, formatServerSummary(server)), m_candidateList);
        item->setData(Qt::UserRole, it.key());
    }
}

void McpServerManagerWindow::renderRegisteredList()
{
    m_registeredList->clear();

    for (auto it = m_registeredById.constBegin(); it != m_registeredById.constEnd(); ++it) {
        const qtllm::tools::mcp::McpServerDefinition &server = it.value();
        auto *item = new QListWidgetItem(formatServerSummary(server), m_registeredList);
        item->setData(Qt::UserRole, it.key());
    }
}

void McpServerManagerWindow::setSelectedServer(const qtllm::tools::mcp::McpServerDefinition &server)
{
    m_selectedServer = server;
    m_hasSelectedServer = true;

    m_detailTitle->setText(QStringLiteral("Selected: %1").arg(server.serverId));

    QStringList lines;
    lines.append(QStringLiteral("serverId: ") + server.serverId);
    lines.append(QStringLiteral("name: ") + server.name);
    lines.append(QStringLiteral("transport: ") + server.transport);
    lines.append(QStringLiteral("enabled: ") + QString(server.enabled ? QStringLiteral("true") : QStringLiteral("false")));
    lines.append(QStringLiteral("command: ") + server.command);
    lines.append(QStringLiteral("args: ") + server.args.join(QStringLiteral(", ")));
    lines.append(QStringLiteral("url: ") + server.url);
    lines.append(QStringLiteral("timeoutMs: ") + QString::number(server.timeoutMs));
    lines.append(QStringLiteral("env: ") + QString::fromUtf8(QJsonDocument(server.env).toJson(QJsonDocument::Compact)));
    lines.append(QStringLiteral("headers: ") + QString::fromUtf8(QJsonDocument(server.headers).toJson(QJsonDocument::Compact)));
    m_detailMeta->setPlainText(lines.join(QStringLiteral("\n")));

    refreshDetails(server);
}

void McpServerManagerWindow::clearDetails()
{
    m_hasSelectedServer = false;
    m_selectedServer = qtllm::tools::mcp::McpServerDefinition();
    m_detailTitle->setText(QStringLiteral("No server selected"));
    m_detailMeta->clear();
    m_toolsView->clear();
    m_resourcesView->clear();
    m_promptsView->clear();
}

void McpServerManagerWindow::refreshDetails(const qtllm::tools::mcp::McpServerDefinition &server)
{
    QString toolsError;
    const QVector<qtllm::tools::mcp::McpToolDescriptor> tools = m_mcpClient->listTools(server, &toolsError);

    if (!toolsError.isEmpty() && tools.isEmpty()) {
        m_toolsView->setPlainText(QStringLiteral("Failed or unsupported:\n") + toolsError);
    } else {
        QStringList lines;
        for (const qtllm::tools::mcp::McpToolDescriptor &tool : tools) {
            lines.append(QStringLiteral("- ") + tool.name + QStringLiteral(": ") + tool.description);
            lines.append(QStringLiteral("  schema: ") + QString::fromUtf8(QJsonDocument(tool.inputSchema).toJson(QJsonDocument::Compact)));
        }
        if (lines.isEmpty()) {
            lines.append(QStringLiteral("No tools returned."));
        }
        m_toolsView->setPlainText(lines.join(QStringLiteral("\n")));
    }

    QString resourcesError;
    const QVector<qtllm::tools::mcp::McpResourceDescriptor> resources = m_mcpClient->listResources(server, &resourcesError);
    if (!resourcesError.isEmpty() && resources.isEmpty()) {
        m_resourcesView->setPlainText(QStringLiteral("Failed or unsupported:\n") + resourcesError);
    } else {
        QStringList lines;
        for (const qtllm::tools::mcp::McpResourceDescriptor &resource : resources) {
            lines.append(QStringLiteral("- ") + resource.name + QStringLiteral(" [") + resource.uri + QStringLiteral("]"));
            if (!resource.description.isEmpty()) {
                lines.append(QStringLiteral("  ") + resource.description);
            }
        }
        if (lines.isEmpty()) {
            lines.append(QStringLiteral("No resources returned."));
        }
        m_resourcesView->setPlainText(lines.join(QStringLiteral("\n")));
    }

    QString promptsError;
    const QVector<qtllm::tools::mcp::McpPromptDescriptor> prompts = m_mcpClient->listPrompts(server, &promptsError);
    if (!promptsError.isEmpty() && prompts.isEmpty()) {
        m_promptsView->setPlainText(QStringLiteral("Failed or unsupported:\n") + promptsError);
    } else {
        QStringList lines;
        for (const qtllm::tools::mcp::McpPromptDescriptor &prompt : prompts) {
            lines.append(QStringLiteral("- ") + prompt.name + QStringLiteral(": ") + prompt.description);
            lines.append(QStringLiteral("  args: ") + QString::fromUtf8(QJsonDocument(prompt.arguments).toJson(QJsonDocument::Compact)));
        }
        if (lines.isEmpty()) {
            lines.append(QStringLiteral("No prompts returned."));
        }
        m_promptsView->setPlainText(lines.join(QStringLiteral("\n")));
    }

    appendLog(QStringLiteral("Details refreshed for: ") + server.serverId);
}

void McpServerManagerWindow::onLogEventReceived(const qtllm::logging::LogEvent &event)
{
    if (!event.category.startsWith(QStringLiteral("mcp."))) {
        return;
    }

    m_logView->append(formatLogEvent(event));
}

void McpServerManagerWindow::appendLog(const QString &line)
{
    m_logView->append(line);
    qtllm::logging::QtLlmLogger::instance().info(QStringLiteral("ui.mcp.manager"), line);
}

QVector<qtllm::tools::mcp::McpServerDefinition> McpServerManagerWindow::scanPossibleServers() const
{
    QVector<qtllm::tools::mcp::McpServerDefinition> servers;

    const QString home = QDir::homePath();
    const QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    const QStringList files = {
        QDir::current().filePath(QStringLiteral(".qtllm/mcp/servers.json")),
        QDir::current().filePath(QStringLiteral("mcp_servers.json")),
        QDir::current().filePath(QStringLiteral(".cursor/mcp.json")),
        QDir::current().filePath(QStringLiteral(".vscode/mcp.json")),
        QDir(home).filePath(QStringLiteral(".cursor/mcp.json")),
        QDir(home).filePath(QStringLiteral(".vscode/mcp.json")),
        QDir(home).filePath(QStringLiteral(".claude/claude_desktop_config.json")),
        QDir(appData).filePath(QStringLiteral("Claude/claude_desktop_config.json"))
    };

    QHash<QString, qtllm::tools::mcp::McpServerDefinition> dedup;

    for (const QString &path : files) {
        QFile file(path);
        if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        file.close();

        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            continue;
        }

        const QVector<qtllm::tools::mcp::McpServerDefinition> parsed = parseServersFromJson(doc.object());
        for (const qtllm::tools::mcp::McpServerDefinition &server : parsed) {
            const QString id = normalizeId(server.serverId);
            if (!id.isEmpty()) {
                dedup.insert(id, server);
            }
        }
    }

    for (auto it = dedup.constBegin(); it != dedup.constEnd(); ++it) {
        servers.append(it.value());
    }

    return servers;
}

QVector<qtllm::tools::mcp::McpServerDefinition> McpServerManagerWindow::parseServersFromJson(const QJsonObject &root)
{
    QVector<qtllm::tools::mcp::McpServerDefinition> out;

    const QJsonArray serversArray = root.value(QStringLiteral("servers")).toArray();
    for (const QJsonValue &v : serversArray) {
        const qtllm::tools::mcp::McpServerDefinition server =
            qtllm::tools::mcp::McpServerDefinition::fromJson(v.toObject());
        if (!normalizeId(server.serverId).isEmpty()) {
            out.append(server);
        }
    }

    const QJsonObject mcpServers = root.value(QStringLiteral("mcpServers")).toObject();
    for (auto it = mcpServers.constBegin(); it != mcpServers.constEnd(); ++it) {
        const QJsonObject item = it.value().toObject();
        qtllm::tools::mcp::McpServerDefinition server;
        server.serverId = normalizeId(it.key());
        server.name = item.value(QStringLiteral("name")).toString(server.serverId);
        server.transport = item.value(QStringLiteral("transport")).toString();
        if (server.transport.isEmpty()) {
            server.transport = item.value(QStringLiteral("url")).toString().isEmpty()
                ? QStringLiteral("stdio")
                : QStringLiteral("streamable-http");
        }
        server.command = item.value(QStringLiteral("command")).toString();
        server.url = item.value(QStringLiteral("url")).toString();
        server.env = item.value(QStringLiteral("env")).toObject();
        server.headers = item.value(QStringLiteral("headers")).toObject();
        server.enabled = item.value(QStringLiteral("enabled")).toBool(true);
        server.timeoutMs = item.value(QStringLiteral("timeoutMs")).toInt(30000);

        const QJsonArray args = item.value(QStringLiteral("args")).toArray();
        for (const QJsonValue &av : args) {
            const QString arg = av.toString();
            if (!arg.isEmpty()) {
                server.args.append(arg);
            }
        }

        if (!server.serverId.isEmpty()) {
            out.append(server);
        }
    }

    return out;
}

bool McpServerManagerWindow::maybeAvailable(const qtllm::tools::mcp::McpServerDefinition &server)
{
    const QString transport = server.transport.trimmed().toLower();
    if (transport == QStringLiteral("stdio")) {
        if (server.command.trimmed().isEmpty()) {
            return false;
        }
        if (QFile::exists(server.command)) {
            return true;
        }
        return !QStandardPaths::findExecutable(server.command).isEmpty();
    }

    if (transport == QStringLiteral("streamable-http") || transport == QStringLiteral("sse")) {
        const QUrl url(server.url);
        return url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty();
    }

    return false;
}

qtllm::tools::mcp::McpServerDefinition McpServerManagerWindow::buildManualServer(
    const QString &serverId,
    const QString &name,
    const QString &transport,
    const QString &command,
    const QString &args,
    const QString &url,
    const QString &envJson,
    const QString &headersJson,
    int timeoutMs,
    QString *errorMessage)
{
    qtllm::tools::mcp::McpServerDefinition server;
    server.serverId = normalizeId(serverId);
    server.name = name.trimmed().isEmpty() ? server.serverId : name.trimmed();
    server.transport = transport.trimmed().toLower();
    server.command = command.trimmed();
    server.args = parseArgs(args);
    server.url = url.trimmed();
    server.timeoutMs = timeoutMs;
    server.enabled = true;

    QString parseError;
    server.env = parseJsonObjectText(envJson, &parseError);
    if (!parseError.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid env JSON. ") + parseError;
        }
        return qtllm::tools::mcp::McpServerDefinition();
    }

    parseError.clear();
    server.headers = parseJsonObjectText(headersJson, &parseError);
    if (!parseError.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid headers JSON. ") + parseError;
        }
        return qtllm::tools::mcp::McpServerDefinition();
    }

    if (!qtllm::tools::mcp::McpServerRegistry::isValidServerId(server.serverId)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Invalid serverId. Use 1-32 chars: [a-z0-9_-]");
        }
        return qtllm::tools::mcp::McpServerDefinition();
    }

    if (server.transport == QStringLiteral("stdio") && server.command.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("stdio requires command");
        }
        return qtllm::tools::mcp::McpServerDefinition();
    }

    if ((server.transport == QStringLiteral("streamable-http") || server.transport == QStringLiteral("sse"))
        && server.url.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("HTTP/SSE requires url");
        }
        return qtllm::tools::mcp::McpServerDefinition();
    }

    if (errorMessage) {
        errorMessage->clear();
    }
    return server;
}



















