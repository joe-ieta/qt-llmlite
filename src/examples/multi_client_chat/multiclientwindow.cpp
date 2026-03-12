#include "multiclientwindow.h"

#include "../../qtllm/chat/conversationclient.h"
#include "../../qtllm/chat/conversationclientfactory.h"
#include "../../qtllm/core/llmconfig.h"
#include "../../qtllm/core/llmtypes.h"
#include "../../qtllm/profile/clientprofile.h"
#include "../../qtllm/providers/illmprovider.h"
#include "../../qtllm/providers/ollamaprovider.h"
#include "../../qtllm/providers/openaicompatibleprovider.h"
#include "../../qtllm/providers/vllmprovider.h"
#include "../../qtllm/storage/conversationrepository.h"
#include "../../qtllm/tools/builtintools.h"
#include "../../qtllm/tools/llmtoolregistry.h"
#include "../../qtllm/tools/toolenabledchatentry.h"
#include "../../qtllm/tools/runtime/clienttoolpolicyrepository.h"
#include "../../qtllm/tools/runtime/toolcatalogrepository.h"
#include "../../qtllm/tools/runtime/toolruntime_types.h"
#include "../../qtllm/tools/mcp/mcpservermanager.h"
#include "../../qtllm/tools/mcp/mcptoolsyncservice.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QSplitter>
#include <QTextCursor>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

#include <memory>
#include <optional>

namespace {

struct ProviderOption
{
    QString id;
    QString title;
    QString defaultBaseUrl;
    bool apiKeyRequired = false;
};

const QList<ProviderOption> &providerOptions()
{
    static const QList<ProviderOption> options = {
        {QStringLiteral("ollama"), QStringLiteral("Ollama"), QStringLiteral("http://127.0.0.1:11434/v1"), false},
        {QStringLiteral("vllm"), QStringLiteral("vLLM"), QStringLiteral("http://127.0.0.1:8000/v1"), false},
        {QStringLiteral("sglang"), QStringLiteral("SGLang"), QStringLiteral("http://127.0.0.1:30000/v1"), false},
        {QStringLiteral("openai"), QStringLiteral("OpenAI"), QStringLiteral("https://api.openai.com/v1"), true},
        {QStringLiteral("openai-compatible"), QStringLiteral("OpenAI-Compatible"), QStringLiteral("http://127.0.0.1:11434/v1"), false},
    };
    return options;
}

const ProviderOption *findProviderOption(const QString &providerId)
{
    const QList<ProviderOption> &options = providerOptions();
    for (const ProviderOption &option : options) {
        if (option.id == providerId) {
            return &option;
        }
    }
    return nullptr;
}

std::unique_ptr<qtllm::ILLMProvider> createProviderById(const QString &providerId)
{
    if (providerId == QStringLiteral("ollama")) {
        return std::make_unique<qtllm::OllamaProvider>();
    }
    if (providerId == QStringLiteral("vllm")) {
        return std::make_unique<qtllm::VllmProvider>();
    }
    return std::make_unique<qtllm::OpenAICompatibleProvider>();
}

QUrl buildModelsUrl(const QString &baseUrl)
{
    QUrl url(baseUrl.trimmed());
    QString path = url.path();
    if (!path.endsWith(QStringLiteral("/models"))) {
        if (!path.endsWith('/')) {
            path += '/';
        }
        path += QStringLiteral("models");
        url.setPath(path);
    }
    return url;
}

QStringList parseTags(const QString &input)
{
    QStringList parts = input.split(',', Qt::SkipEmptyParts);
    for (QString &part : parts) {
        part = part.trimmed();
    }
    parts.removeAll(QString());
    return parts;
}

QString joinTags(const QStringList &tags)
{
    return tags.join(QStringLiteral(", "));
}

void ensureDefaultTools(const std::shared_ptr<qtllm::tools::LlmToolRegistry> &registry, bool *changed)
{
    if (!registry) {
        return;
    }

    bool localChanged = false;
    const QList<qtllm::tools::LlmToolDefinition> merged =
        qtllm::tools::mergeWithBuiltInTools(registry->allTools(), &localChanged);
    if (localChanged) {
        registry->replaceAll(merged);
    }

    if (changed) {
        *changed = localChanged;
    }
}

} // namespace

MultiClientWindow::MultiClientWindow(QWidget *parent)
    : QWidget(parent)
    , m_repository(std::make_shared<qtllm::storage::ConversationRepository>(QStringLiteral(".qtllm/clients")))
    , m_factory(std::make_unique<qtllm::chat::ConversationClientFactory>())
    , m_toolRegistry(std::make_shared<qtllm::tools::LlmToolRegistry>())
    , m_toolCatalogRepository(std::make_shared<qtllm::tools::runtime::ToolCatalogRepository>())
    , m_clientPolicyRepository(std::make_shared<qtllm::tools::runtime::ClientToolPolicyRepository>())
    , m_mcpServerManager(std::make_shared<qtllm::tools::mcp::McpServerManager>())
    , m_mcpToolSyncService(std::make_shared<qtllm::tools::mcp::McpToolSyncService>(m_toolRegistry, m_mcpServerManager ? m_mcpServerManager->registry() : nullptr))
    , m_toolEntry(nullptr)
    , m_clientList(new QListWidget(this))
    , m_newClientButton(new QPushButton(QStringLiteral("New Client"), this))
    , m_sessionList(new QListWidget(this))
    , m_newSessionButton(new QPushButton(QStringLiteral("New Session"), this))
    , m_output(new QTextEdit(this))
    , m_input(new QTextEdit(this))
    , m_useToolsCheck(new QCheckBox(QStringLiteral("Use Tools Entry"), this))
    , m_sendButton(new QPushButton(QStringLiteral("Send"), this))
    , m_providerCombo(new QComboBox(this))
    , m_baseUrlEdit(new QLineEdit(this))
    , m_apiKeyEdit(new QLineEdit(this))
    , m_modelCombo(new QComboBox(this))
    , m_reloadModelsButton(new QPushButton(QStringLiteral("Refresh Models"), this))
    , m_applyConfigButton(new QPushButton(QStringLiteral("Apply Config"), this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_systemPromptEdit(new QTextEdit(this))
    , m_personaEdit(new QLineEdit(this))
    , m_thinkingStyleEdit(new QLineEdit(this))
    , m_skillsEdit(new QLineEdit(this))
    , m_capabilitiesEdit(new QLineEdit(this))
    , m_preferencesEdit(new QLineEdit(this))
    , m_historyWindowEdit(new QLineEdit(this))
    , m_applyProfileButton(new QPushButton(QStringLiteral("Apply Profile"), this))
{
    m_factory->setRepository(m_repository);
    bool changedCatalog = false;
    if (m_toolCatalogRepository) {
        const std::optional<qtllm::tools::runtime::ToolCatalogSnapshot> loaded =
            m_toolCatalogRepository->loadCatalog(nullptr);
        if (loaded.has_value()) {
            QList<qtllm::tools::LlmToolDefinition> loadedTools;
            for (const qtllm::tools::LlmToolDefinition &tool : loaded->tools) {
                loadedTools.append(tool);
            }
            m_toolRegistry->replaceAll(loadedTools);
        }
    }

    ensureDefaultTools(m_toolRegistry, &changedCatalog);

    if (m_mcpServerManager) {
        m_mcpServerManager->load(nullptr);
    }

    if (m_mcpToolSyncService && m_mcpServerManager) {
        const QVector<qtllm::tools::mcp::McpServerDefinition> servers = m_mcpServerManager->allServers();
        for (const qtllm::tools::mcp::McpServerDefinition &server : servers) {
            if (!server.enabled) {
                continue;
            }
            if (m_mcpToolSyncService->syncServerTools(server.serverId, nullptr)) {
                changedCatalog = true;
            }
        }
    }

    if (changedCatalog && m_toolCatalogRepository) {
        qtllm::tools::runtime::ToolCatalogSnapshot snapshot;
        const QList<qtllm::tools::LlmToolDefinition> allTools = m_toolRegistry->allTools();
        for (const qtllm::tools::LlmToolDefinition &tool : allTools) {
            snapshot.tools.append(tool);
        }
        m_toolCatalogRepository->saveCatalog(snapshot, nullptr);
    }

    m_systemPromptEdit->setPlaceholderText(QStringLiteral("System prompt"));
    m_output->setReadOnly(true);
    m_input->setPlaceholderText(QStringLiteral("Input message"));
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_modelCombo->setEditable(false);

    for (const ProviderOption &option : providerOptions()) {
        m_providerCombo->addItem(option.title, option.id);
    }

    auto *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(new QLabel(QStringLiteral("Clients"), this));
    leftLayout->addWidget(m_clientList, 1);
    leftLayout->addWidget(m_newClientButton);
    leftLayout->addWidget(new QLabel(QStringLiteral("Sessions"), this));
    leftLayout->addWidget(m_sessionList, 1);
    leftLayout->addWidget(m_newSessionButton);
    auto *leftPane = new QWidget(this);
    leftPane->setLayout(leftLayout);

    auto *centerLayout = new QVBoxLayout();
    centerLayout->addWidget(m_output, 1);
    centerLayout->addWidget(m_input);
    centerLayout->addWidget(m_useToolsCheck);
    centerLayout->addWidget(m_sendButton);

    auto *modelRowLayout = new QHBoxLayout();
    modelRowLayout->addWidget(m_modelCombo, 1);
    modelRowLayout->addWidget(m_reloadModelsButton, 0);

    auto *configForm = new QFormLayout();
    configForm->addRow(QStringLiteral("Provider"), m_providerCombo);
    configForm->addRow(QStringLiteral("Base URL"), m_baseUrlEdit);
    configForm->addRow(QStringLiteral("API Key"), m_apiKeyEdit);
    configForm->addRow(QStringLiteral("Model"), modelRowLayout);

    auto *profileForm = new QFormLayout();
    profileForm->addRow(QStringLiteral("System"), m_systemPromptEdit);
    profileForm->addRow(QStringLiteral("Persona"), m_personaEdit);
    profileForm->addRow(QStringLiteral("Thinking"), m_thinkingStyleEdit);
    profileForm->addRow(QStringLiteral("Skills"), m_skillsEdit);
    profileForm->addRow(QStringLiteral("Capabilities"), m_capabilitiesEdit);
    profileForm->addRow(QStringLiteral("Preferences"), m_preferencesEdit);
    profileForm->addRow(QStringLiteral("History Window"), m_historyWindowEdit);

    auto *rightLayout = new QVBoxLayout();
    rightLayout->addLayout(configForm);
    rightLayout->addWidget(m_applyConfigButton);
    rightLayout->addSpacing(8);
    rightLayout->addLayout(profileForm);
    rightLayout->addWidget(m_applyProfileButton);
    auto *rightPane = new QWidget(this);
    rightPane->setLayout(rightLayout);

    auto *centerPane = new QWidget(this);
    centerPane->setLayout(centerLayout);

    auto *splitter = new QSplitter(this);
    splitter->addWidget(leftPane);
    splitter->addWidget(centerPane);
    splitter->addWidget(rightPane);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 0);

    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->addWidget(splitter);
    setLayout(rootLayout);

    setWindowTitle(QStringLiteral("qt-llm multi client chat"));
    resize(1280, 760);

    connect(m_newClientButton, &QPushButton::clicked, this, &MultiClientWindow::onNewClientClicked);
    connect(m_clientList, &QListWidget::itemSelectionChanged, this, &MultiClientWindow::onClientSelectionChanged);
    connect(m_newSessionButton, &QPushButton::clicked, this, &MultiClientWindow::onNewSessionClicked);
    connect(m_sessionList, &QListWidget::itemSelectionChanged, this, &MultiClientWindow::onSessionSelectionChanged);
    connect(m_sendButton, &QPushButton::clicked, this, &MultiClientWindow::onSendClicked);
    connect(m_applyProfileButton, &QPushButton::clicked, this, &MultiClientWindow::onApplyProfileClicked);
    connect(m_applyConfigButton, &QPushButton::clicked, this, &MultiClientWindow::onApplyConfigClicked);
    connect(m_reloadModelsButton, &QPushButton::clicked, this, &MultiClientWindow::onRefreshModelsClicked);
    connect(m_providerCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) {
        const QString providerId = m_providerCombo->currentData().toString();
        const ProviderOption *option = findProviderOption(providerId);
        if (option) {
            m_baseUrlEdit->setText(option->defaultBaseUrl);
        }
        refreshModels();
    });
    connect(m_baseUrlEdit, &QLineEdit::editingFinished, this, &MultiClientWindow::onRefreshModelsClicked);
    connect(m_apiKeyEdit, &QLineEdit::editingFinished, this, &MultiClientWindow::onRefreshModelsClicked);

    loadClients();
}

QSharedPointer<qtllm::chat::ConversationClient> MultiClientWindow::activeClient() const
{
    const QListWidgetItem *item = m_clientList->currentItem();
    if (!item) {
        return {};
    }

    const QString clientId = item->data(Qt::UserRole).toString();
    return m_factory->find(clientId);
}

void MultiClientWindow::loadClients()
{
    const QStringList persistedIds = m_factory->persistedClientIds();
    for (const QString &clientId : persistedIds) {
        m_factory->acquire(clientId);
        addClientToList(clientId);
    }

    if (m_clientList->count() == 0) {
        onNewClientClicked();
        return;
    }

    m_clientList->setCurrentRow(0);
    onClientSelectionChanged();
}

void MultiClientWindow::addClientToList(const QString &clientId)
{
    if (clientId.isEmpty()) {
        return;
    }

    for (int i = 0; i < m_clientList->count(); ++i) {
        if (m_clientList->item(i)->data(Qt::UserRole).toString() == clientId) {
            return;
        }
    }

    auto *item = new QListWidgetItem(clientId, m_clientList);
    item->setData(Qt::UserRole, clientId);
}

void MultiClientWindow::bindActiveClient(const QSharedPointer<qtllm::chat::ConversationClient> &client)
{
    QObject::disconnect(m_tokenConn);
    QObject::disconnect(m_completedConn);
    QObject::disconnect(m_errorConn);
    QObject::disconnect(m_historyConn);
    QObject::disconnect(m_sessionsConn);
    QObject::disconnect(m_activeSessionConn);

    if (!client) {
        return;
    }

    m_tokenConn = connect(client.get(), &qtllm::chat::ConversationClient::tokenReceived, this, [this](const QString &token) {
        m_output->moveCursor(QTextCursor::End);
        m_output->insertPlainText(token);
    });

    m_completedConn = connect(client.get(), &qtllm::chat::ConversationClient::completed, this, [this](const QString &) {
        m_output->append(QString());
        m_output->append(QStringLiteral("--- done ---"));
    });

    m_errorConn = connect(client.get(), &qtllm::chat::ConversationClient::errorOccurred, this, [this](const QString &message) {
        m_output->append(QStringLiteral("[error] ") + message);
    });

    m_historyConn = connect(client.get(), &qtllm::chat::ConversationClient::historyChanged, this, [this]() {
        renderActiveHistory(activeClient());
    });

    m_sessionsConn = connect(client.get(), &qtllm::chat::ConversationClient::sessionsChanged, this, [this, client]() {
        rebuildSessionList(client);
    });

    m_activeSessionConn = connect(client.get(), &qtllm::chat::ConversationClient::activeSessionChanged, this, [this, client](const QString &) {
        rebuildSessionList(client);
        renderActiveHistory(client);
    });
}

void MultiClientWindow::rebuildSessionList(const QSharedPointer<qtllm::chat::ConversationClient> &client)
{
    m_sessionList->clear();
    if (!client) {
        return;
    }

    const QString currentSessionId = client->activeSessionId();
    int currentRow = -1;

    const QStringList ids = client->sessionIds();
    for (int i = 0; i < ids.size(); ++i) {
        const QString sessionId = ids.at(i);
        const QString title = client->sessionTitle(sessionId);
        const QString label = QStringLiteral("%1 (%2)").arg(title, sessionId.left(8));
        auto *item = new QListWidgetItem(label, m_sessionList);
        item->setData(Qt::UserRole, sessionId);
        if (sessionId == currentSessionId) {
            currentRow = i;
        }
    }

    if (currentRow >= 0) {
        m_sessionList->setCurrentRow(currentRow);
    }
}

void MultiClientWindow::renderActiveHistory(const QSharedPointer<qtllm::chat::ConversationClient> &client)
{
    m_output->clear();
    if (!client) {
        return;
    }

    const QVector<qtllm::LlmMessage> history = client->history();
    for (const qtllm::LlmMessage &message : history) {
        if (message.role == QStringLiteral("user")) {
            m_output->append(QStringLiteral("> ") + message.content);
        } else if (message.role == QStringLiteral("assistant")) {
            m_output->append(message.content);
            m_output->append(QStringLiteral("--- done ---"));
        } else {
            m_output->append(QStringLiteral("[") + message.role + QStringLiteral("] ") + message.content);
        }
    }
}

void MultiClientWindow::refreshEditorFromActiveClient(const QSharedPointer<qtllm::chat::ConversationClient> &client)
{
    if (!client) {
        return;
    }

    const qtllm::LlmConfig config = client->config();
    int providerIndex = m_providerCombo->findData(config.providerName);
    if (providerIndex < 0) {
        providerIndex = 0;
    }

    m_providerCombo->setCurrentIndex(providerIndex);

    if (config.baseUrl.isEmpty() && providerIndex >= 0) {
        m_baseUrlEdit->setText(providerOptions().at(providerIndex).defaultBaseUrl);
    } else {
        m_baseUrlEdit->setText(config.baseUrl);
    }

    m_apiKeyEdit->setText(config.apiKey);

    m_modelCombo->clear();
    if (!config.model.isEmpty()) {
        m_modelCombo->addItem(config.model, config.model);
        m_modelCombo->setCurrentIndex(0);
    }

    const qtllm::profile::ClientProfile profile = client->profile();
    m_systemPromptEdit->setPlainText(profile.systemPrompt);
    m_personaEdit->setText(profile.persona);
    m_thinkingStyleEdit->setText(profile.thinkingStyle);
    m_skillsEdit->setText(joinTags(profile.skillIds));
    m_capabilitiesEdit->setText(joinTags(profile.capabilityTags));
    m_preferencesEdit->setText(joinTags(profile.preferenceTags));
    m_historyWindowEdit->setText(QString::number(profile.memoryPolicy.maxHistoryMessages));

    refreshModels();
}

bool MultiClientWindow::applyConfigToActiveClient(bool showMessage)
{
    const QSharedPointer<qtllm::chat::ConversationClient> client = activeClient();
    if (!client) {
        return false;
    }

    const QString providerId = m_providerCombo->currentData().toString();
    const ProviderOption *option = findProviderOption(providerId);
    if (!option) {
        if (showMessage) {
            m_output->append(QStringLiteral("[config] Invalid provider"));
        }
        return false;
    }

    QString baseUrl = m_baseUrlEdit->text().trimmed();
    if (baseUrl.isEmpty()) {
        baseUrl = option->defaultBaseUrl;
        m_baseUrlEdit->setText(baseUrl);
    }

    const QUrl parsedUrl(baseUrl);
    if (!parsedUrl.isValid() || parsedUrl.scheme().isEmpty() || parsedUrl.host().isEmpty()) {
        if (showMessage) {
            m_output->append(QStringLiteral("[config] Invalid base URL"));
        }
        return false;
    }

    const QString apiKey = m_apiKeyEdit->text().trimmed();
    if (option->apiKeyRequired && apiKey.isEmpty()) {
        if (showMessage) {
            m_output->append(QStringLiteral("[config] Provider requires API key"));
        }
        return false;
    }

    const QString modelId = m_modelCombo->currentText().trimmed();
    if (modelId.isEmpty()) {
        if (showMessage) {
            m_output->append(QStringLiteral("[config] No model list available, click Refresh Models"));
        }
        return false;
    }

    qtllm::LlmConfig config = client->config();
    config.providerName = providerId;
    config.baseUrl = baseUrl;
    config.apiKey = apiKey;
    config.model = modelId;
    config.stream = true;

    client->setConfig(config);
    client->setProvider(createProviderById(providerId));
    return true;
}

void MultiClientWindow::refreshModels()
{
    const QString providerId = m_providerCombo->currentData().toString();
    const ProviderOption *option = findProviderOption(providerId);
    if (!option) {
        m_output->append(QStringLiteral("[models] Invalid provider"));
        return;
    }

    if (option->apiKeyRequired && m_apiKeyEdit->text().trimmed().isEmpty()) {
        m_modelCombo->clear();
        m_output->append(QStringLiteral("[models] This provider requires API Key before refreshing models"));
        return;
    }

    const QUrl modelsUrl = buildModelsUrl(m_baseUrlEdit->text());
    if (!modelsUrl.isValid() || modelsUrl.scheme().isEmpty() || modelsUrl.host().isEmpty()) {
        m_modelCombo->clear();
        m_output->append(QStringLiteral("[models] Invalid API URL, cannot fetch models"));
        return;
    }

    if (m_modelsReply) {
        m_modelsReply->deleteLater();
        m_modelsReply.clear();
    }

    const QString previousModel = m_modelCombo->currentText();
    m_modelCombo->clear();

    QNetworkRequest request(modelsUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    const QString apiKey = m_apiKeyEdit->text().trimmed();
    if (!apiKey.isEmpty()) {
        request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());
    }

    m_modelsReply = m_networkManager->get(request);
    m_modelsReply->setProperty("previousModel", previousModel);
    connect(m_modelsReply, &QNetworkReply::finished, this, &MultiClientWindow::onModelsReplyFinished);

    m_output->append(QStringLiteral("[models] Fetching model list..."));
}

void MultiClientWindow::rebuildToolEntryForActiveClient(const QSharedPointer<qtllm::chat::ConversationClient> &client)
{
    if (m_toolEntry) {
        m_toolEntry->deleteLater();
        m_toolEntry = nullptr;
    }

    if (!client || !m_toolRegistry) {
        return;
    }

    m_toolEntry = new qtllm::tools::ToolEnabledChatEntry(client, m_toolRegistry, this);
    if (m_clientPolicyRepository) {
        m_toolEntry->setClientPolicyRepository(m_clientPolicyRepository);
    }
    if (m_mcpServerManager) {
        m_toolEntry->setMcpServerRegistry(m_mcpServerManager->registry());
    }
}

void MultiClientWindow::onNewClientClicked()
{
    const QSharedPointer<qtllm::chat::ConversationClient> client = m_factory->acquire();
    addClientToList(client->uid());

    for (int i = 0; i < m_clientList->count(); ++i) {
        if (m_clientList->item(i)->data(Qt::UserRole).toString() == client->uid()) {
            m_clientList->setCurrentRow(i);
            break;
        }
    }

    onClientSelectionChanged();
}

void MultiClientWindow::onClientSelectionChanged()
{
    const QSharedPointer<qtllm::chat::ConversationClient> client = activeClient();
    bindActiveClient(client);
    rebuildSessionList(client);
    renderActiveHistory(client);
    refreshEditorFromActiveClient(client);
    rebuildToolEntryForActiveClient(client);
}

void MultiClientWindow::onNewSessionClicked()
{
    const QSharedPointer<qtllm::chat::ConversationClient> client = activeClient();
    if (!client) {
        return;
    }

    const QString title = QStringLiteral("Session %1").arg(client->sessionIds().size() + 1);
    client->createSession(title);
}

void MultiClientWindow::onSessionSelectionChanged()
{
    const QSharedPointer<qtllm::chat::ConversationClient> client = activeClient();
    if (!client || !m_sessionList->currentItem()) {
        return;
    }

    const QString sessionId = m_sessionList->currentItem()->data(Qt::UserRole).toString();
    if (client->switchSession(sessionId)) {
        renderActiveHistory(client);
    }
}

void MultiClientWindow::onSendClicked()
{
    const QSharedPointer<qtllm::chat::ConversationClient> client = activeClient();
    if (!client) {
        return;
    }

    const QString prompt = m_input->toPlainText().trimmed();
    if (prompt.isEmpty()) {
        return;
    }

    if (!applyConfigToActiveClient(true)) {
        return;
    }

    m_output->append(QStringLiteral("> ") + prompt);
    m_input->clear();

    if (m_useToolsCheck->isChecked() && m_toolEntry) {
        m_toolEntry->sendUserMessage(prompt);
        return;
    }

    client->sendUserMessage(prompt);
}

void MultiClientWindow::onApplyProfileClicked()
{
    const QSharedPointer<qtllm::chat::ConversationClient> client = activeClient();
    if (!client) {
        return;
    }

    qtllm::profile::ClientProfile profile = client->profile();
    profile.systemPrompt = m_systemPromptEdit->toPlainText().trimmed();
    profile.persona = m_personaEdit->text().trimmed();
    profile.thinkingStyle = m_thinkingStyleEdit->text().trimmed();
    profile.skillIds = parseTags(m_skillsEdit->text());
    profile.capabilityTags = parseTags(m_capabilitiesEdit->text());
    profile.preferenceTags = parseTags(m_preferencesEdit->text());

    bool ok = false;
    const int maxHistory = m_historyWindowEdit->text().toInt(&ok);
    if (ok && maxHistory > 0) {
        profile.memoryPolicy.maxHistoryMessages = maxHistory;
    }

    client->setProfile(profile);
}

void MultiClientWindow::onApplyConfigClicked()
{
    applyConfigToActiveClient(true);
}

void MultiClientWindow::onRefreshModelsClicked()
{
    refreshModels();
}

void MultiClientWindow::onModelsReplyFinished()
{
    if (!m_modelsReply) {
        return;
    }

    const QByteArray payload = m_modelsReply->readAll();
    const QNetworkReply::NetworkError error = m_modelsReply->error();
    const QString errorText = m_modelsReply->errorString();
    const QString previousModel = m_modelsReply->property("previousModel").toString();

    m_modelsReply->deleteLater();
    m_modelsReply.clear();

    if (error != QNetworkReply::NoError) {
        m_output->append(QStringLiteral("[models] Failed to fetch model list: ") + errorText);
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isObject()) {
        m_output->append(QStringLiteral("[models] Model list parse failed: response is not a JSON object"));
        return;
    }

    const QJsonArray data = doc.object().value(QStringLiteral("data")).toArray();
    if (data.isEmpty()) {
        m_output->append(QStringLiteral("[models] Model list is empty; check endpoint, API key, or server status"));
        return;
    }

    m_modelCombo->clear();
    for (const QJsonValue &entry : data) {
        const QString modelId = entry.toObject().value(QStringLiteral("id")).toString();
        if (!modelId.isEmpty()) {
            m_modelCombo->addItem(modelId, modelId);
        }
    }

    if (m_modelCombo->count() == 0) {
        m_output->append(QStringLiteral("[models] Model list does not contain usable model IDs"));
        return;
    }

    const int previousIndex = m_modelCombo->findText(previousModel);
    m_modelCombo->setCurrentIndex(previousIndex >= 0 ? previousIndex : 0);
    m_output->append(QStringLiteral("[models] Model list updated"));
}

