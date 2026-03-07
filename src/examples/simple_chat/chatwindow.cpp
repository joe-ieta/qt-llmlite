#include "chatwindow.h"

#include "../../qtllm/core/llmconfig.h"
#include "../../qtllm/core/qtllmclient.h"
#include "../../qtllm/providers/illmprovider.h"
#include "../../qtllm/providers/ollamaprovider.h"
#include "../../qtllm/providers/openaicompatibleprovider.h"
#include "../../qtllm/providers/vllmprovider.h"

#include <QComboBox>
#include <QEvent>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QTextCursor>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>

#include <memory>

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

std::unique_ptr<ILLMProvider> createProviderById(const QString &providerId)
{
    if (providerId == QStringLiteral("ollama")) {
        return std::make_unique<OllamaProvider>();
    }
    if (providerId == QStringLiteral("vllm")) {
        return std::make_unique<VllmProvider>();
    }
    return std::make_unique<OpenAICompatibleProvider>();
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

} // namespace

ChatWindow::ChatWindow(QWidget *parent)
    : QWidget(parent)
    , m_providerCombo(new QComboBox(this))
    , m_baseUrlEdit(new QLineEdit(this))
    , m_apiKeyEdit(new QLineEdit(this))
    , m_modelCombo(new QComboBox(this))
    , m_reloadModelsButton(new QPushButton(QStringLiteral("刷新模型"), this))
    , m_statusLabel(new QLabel(this))
    , m_output(new QTextEdit(this))
    , m_input(new QLineEdit(this))
    , m_sendButton(new QPushButton(QStringLiteral("Send"), this))
    , m_client(new QtLLMClient(this))
    , m_networkManager(new QNetworkAccessManager(this))
{
    m_output->setReadOnly(true);
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText(QStringLiteral("OpenAI 需要 API Key，其它可留空"));
    m_modelCombo->setEditable(false);
    m_input->setPlaceholderText(QStringLiteral("输入问题前会自动校验参数"));
    m_input->installEventFilter(this);

    for (const ProviderOption &option : providerOptions()) {
        m_providerCombo->addItem(option.title, option.id);
    }

    auto *modelRowLayout = new QHBoxLayout();
    modelRowLayout->addWidget(m_modelCombo, 1);
    modelRowLayout->addWidget(m_reloadModelsButton, 0);

    auto *configLayout = new QFormLayout();
    configLayout->addRow(QStringLiteral("LLM引擎:"), m_providerCombo);
    configLayout->addRow(QStringLiteral("LLM地址:"), m_baseUrlEdit);
    configLayout->addRow(QStringLiteral("Api-Key:"), m_apiKeyEdit);
    configLayout->addRow(QStringLiteral("默认模型:"), modelRowLayout);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(configLayout);
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_output, 1);
    layout->addWidget(m_input);
    layout->addWidget(m_sendButton);
    setLayout(layout);

    resize(860, 560);
    setWindowTitle(QStringLiteral("qt-llm simple chat"));

    connect(m_providerCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &ChatWindow::onProviderChanged);
    connect(m_reloadModelsButton, &QPushButton::clicked,
            this, &ChatWindow::onRefreshModelsClicked);
    connect(m_baseUrlEdit, &QLineEdit::editingFinished,
            this, &ChatWindow::onRefreshModelsClicked);
    connect(m_apiKeyEdit, &QLineEdit::editingFinished,
            this, &ChatWindow::onRefreshModelsClicked);
    connect(m_modelCombo, &QComboBox::currentTextChanged,
            this, [this](const QString &) { applyConfigToClient(); });

    connect(m_sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(m_client, &QtLLMClient::tokenReceived, this, [this](const QString &token) {
        m_output->moveCursor(QTextCursor::End);
        m_output->insertPlainText(token);
    });
    connect(m_client, &QtLLMClient::completed, this, [this](const QString &) {
        m_output->append(QString());
        m_output->append(QStringLiteral("--- done ---"));
    });
    connect(m_client, &QtLLMClient::errorOccurred, this, [this](const QString &message) {
        m_output->append(QStringLiteral("[error] ") + message);
    });

    m_providerCombo->setCurrentIndex(0);
    onProviderChanged(m_providerCombo->currentIndex());
}

void ChatWindow::onProviderChanged(int index)
{
    Q_UNUSED(index)

    const QString providerId = selectedProviderId();
    const ProviderOption *option = findProviderOption(providerId);
    if (!option) {
        return;
    }

    m_baseUrlEdit->setText(option->defaultBaseUrl);
    m_modelCombo->clear();
    m_sendButton->setEnabled(false);
    applyConfigToClient();
    refreshModels();
}

void ChatWindow::onRefreshModelsClicked()
{
    refreshModels();
}

void ChatWindow::onModelsReplyFinished()
{
    if (!m_modelsReply) {
        return;
    }

    const QByteArray payload = m_modelsReply->readAll();
    const QNetworkReply::NetworkError error = m_modelsReply->error();
    const QString errorText = m_modelsReply->errorString();

    m_modelsReply->deleteLater();
    m_modelsReply.clear();

    if (error != QNetworkReply::NoError) {
        setStatusMessage(QStringLiteral("模型列表获取失败: ") + errorText, true);
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(payload);
    if (!doc.isObject()) {
        setStatusMessage(QStringLiteral("模型列表解析失败: 返回内容不是 JSON 对象"), true);
        return;
    }

    const QJsonArray data = doc.object().value(QStringLiteral("data")).toArray();
    if (data.isEmpty()) {
        setStatusMessage(QStringLiteral("模型列表为空，请检查地址、API Key 或服务状态"), true);
        return;
    }

    const QString currentModel = m_modelCombo->currentText();
    m_modelCombo->clear();

    for (const QJsonValue &entry : data) {
        const QString modelId = entry.toObject().value(QStringLiteral("id")).toString();
        if (!modelId.isEmpty()) {
            m_modelCombo->addItem(modelId, modelId);
        }
    }

    if (m_modelCombo->count() == 0) {
        setStatusMessage(QStringLiteral("模型列表中没有可用模型 ID"), true);
        return;
    }

    const int previousIndex = m_modelCombo->findText(currentModel);
    m_modelCombo->setCurrentIndex(previousIndex >= 0 ? previousIndex : 0);

    applyConfigToClient();
    setStatusMessage(QStringLiteral("模型列表已更新"), false);
}

void ChatWindow::onSendClicked()
{
    const QString prompt = m_input->text().trimmed();
    if (prompt.isEmpty()) {
        return;
    }

    if (!validateConfig(true)) {
        return;
    }

    applyConfigToClient();

    m_output->append(QStringLiteral("\n> ") + prompt);
    m_input->clear();
    m_client->sendPrompt(prompt);
}

bool ChatWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_input && event->type() == QEvent::FocusIn) {
        validateConfig(true);
    }
    return QWidget::eventFilter(watched, event);
}

void ChatWindow::applyConfigToClient()
{
    LlmConfig config;
    config.providerName = selectedProviderId();
    config.baseUrl = m_baseUrlEdit->text().trimmed();
    config.apiKey = m_apiKeyEdit->text().trimmed();
    config.model = m_modelCombo->currentText().trimmed();
    config.stream = true;

    m_client->setConfig(config);
    m_client->setProvider(createProviderById(config.providerName));
}

bool ChatWindow::validateConfig(bool showMessage)
{
    const QString providerId = selectedProviderId();
    const ProviderOption *option = findProviderOption(providerId);

    QString message;
    bool ok = true;

    const QString baseUrl = m_baseUrlEdit->text().trimmed();
    const QUrl parsedUrl(baseUrl);
    if (!option) {
        ok = false;
        message = QStringLiteral("请选择模型提供商");
    } else if (baseUrl.isEmpty() || !parsedUrl.isValid() || parsedUrl.scheme().isEmpty() || parsedUrl.host().isEmpty()) {
        ok = false;
        message = QStringLiteral("模型提供商 API 地址无效，请输入完整 http/https 地址");
    } else if (option->apiKeyRequired && m_apiKeyEdit->text().trimmed().isEmpty()) {
        ok = false;
        message = QStringLiteral("OpenAI 需要 API Key，请先填写");
    } else if (m_modelCombo->currentText().trimmed().isEmpty()) {
        ok = false;
        message = QStringLiteral("请先加载并选择模型");
    }

    m_sendButton->setEnabled(ok);

    if (ok) {
        if (showMessage) {
            setStatusMessage(QStringLiteral("参数校验通过"), false);
        }
        return true;
    }

    if (showMessage) {
        setStatusMessage(message, true);
        m_output->append(QStringLiteral("[config] ") + message);
    }
    return false;
}

void ChatWindow::setStatusMessage(const QString &message, bool isError)
{
    m_statusLabel->setText(message);
    if (isError) {
        m_statusLabel->setStyleSheet(QStringLiteral("color:#b91c1c;"));
    } else {
        m_statusLabel->setStyleSheet(QStringLiteral("color:#166534;"));
    }
}

void ChatWindow::refreshModels()
{
    applyConfigToClient();

    const QString providerId = selectedProviderId();
    const ProviderOption *option = findProviderOption(providerId);
    if (!option) {
        setStatusMessage(QStringLiteral("无效的模型提供商"), true);
        return;
    }

    if (option->apiKeyRequired && m_apiKeyEdit->text().trimmed().isEmpty()) {
        m_modelCombo->clear();
        setStatusMessage(QStringLiteral("该提供商需要 API Key，填写后再加载模型"), true);
        m_sendButton->setEnabled(false);
        return;
    }

    const QUrl modelsUrl = buildModelsUrl(m_baseUrlEdit->text());
    if (!modelsUrl.isValid() || modelsUrl.scheme().isEmpty() || modelsUrl.host().isEmpty()) {
        setStatusMessage(QStringLiteral("API 地址无效，无法获取模型列表"), true);
        return;
    }

    if (m_modelsReply) {
        m_modelsReply->deleteLater();
        m_modelsReply.clear();
    }

    m_modelCombo->clear();
    m_sendButton->setEnabled(false);

    QNetworkRequest request(modelsUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    const QString apiKey = m_apiKeyEdit->text().trimmed();
    if (!apiKey.isEmpty()) {
        request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());
    }

    m_modelsReply = m_networkManager->get(request);
    connect(m_modelsReply, &QNetworkReply::finished,
            this, &ChatWindow::onModelsReplyFinished);

    setStatusMessage(QStringLiteral("正在获取模型列表..."), false);
}

QString ChatWindow::selectedProviderId() const
{
    return m_providerCombo->currentData().toString();
}
