#include "chatwindow.h"

#include "../../qtllm/core/llmconfig.h"
#include "../../qtllm/core/qtllmclient.h"
#include "../../qtllm/providers/openaicompatibleprovider.h"

#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

ChatWindow::ChatWindow(QWidget *parent)
    : QWidget(parent)
    , m_output(new QTextEdit(this))
    , m_input(new QLineEdit(this))
    , m_sendButton(new QPushButton(QStringLiteral("Send"), this))
    , m_client(new QtLLMClient(this))
{
    m_output->setReadOnly(true);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_output);
    layout->addWidget(m_input);
    layout->addWidget(m_sendButton);
    setLayout(layout);
    resize(720, 480);
    setWindowTitle(QStringLiteral("qt-llm simple chat"));

    LlmConfig config;
    config.providerName = QStringLiteral("openai-compatible");
    config.baseUrl = QStringLiteral("http://127.0.0.1:11434/v1");
    config.model = QStringLiteral("llama3");
    config.stream = true;

    m_client->setConfig(config);
    m_client->setProvider(std::make_unique<OpenAICompatibleProvider>());

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
}

void ChatWindow::onSendClicked()
{
    const QString prompt = m_input->text().trimmed();
    if (prompt.isEmpty()) {
        return;
    }

    m_output->append(QStringLiteral("\n> ") + prompt);
    m_input->clear();
    m_client->sendPrompt(prompt);
}
