#include "manualmcpserverdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

QString normalizeId(const QString &value)
{
    return value.trimmed().toLower();
}

} // namespace

ManualMcpServerDialog::ManualMcpServerDialog(QWidget *parent)
    : QDialog(parent)
    , m_idEdit(new QLineEdit(this))
    , m_nameEdit(new QLineEdit(this))
    , m_transportCombo(new QComboBox(this))
    , m_commandEdit(new QLineEdit(this))
    , m_argsEdit(new QLineEdit(this))
    , m_urlEdit(new QLineEdit(this))
    , m_envEdit(new QTextEdit(this))
    , m_headersEdit(new QTextEdit(this))
    , m_timeoutEdit(new QLineEdit(this))
{
    setWindowTitle(QStringLiteral("Add MCP Server"));
    resize(520, 420);

    m_transportCombo->addItems(QStringList({QStringLiteral("stdio"),
                                            QStringLiteral("streamable-http"),
                                            QStringLiteral("sse")}));
    m_envEdit->setPlaceholderText(QStringLiteral("{}"));
    m_headersEdit->setPlaceholderText(QStringLiteral("{}"));
    m_argsEdit->setPlaceholderText(QStringLiteral("arg1,arg2,arg3"));
    m_timeoutEdit->setText(QStringLiteral("30000"));

    auto *form = new QFormLayout();
    form->addRow(QStringLiteral("Server ID"), m_idEdit);
    form->addRow(QStringLiteral("Name"), m_nameEdit);
    form->addRow(QStringLiteral("Transport"), m_transportCombo);
    form->addRow(QStringLiteral("Command (stdio)"), m_commandEdit);
    form->addRow(QStringLiteral("Args CSV"), m_argsEdit);
    form->addRow(QStringLiteral("URL (http/sse)"), m_urlEdit);
    form->addRow(QStringLiteral("Env JSON"), m_envEdit);
    form->addRow(QStringLiteral("Headers JSON"), m_headersEdit);
    form->addRow(QStringLiteral("Timeout ms"), m_timeoutEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &ManualMcpServerDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ManualMcpServerDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

qtllm::tools::mcp::McpServerDefinition ManualMcpServerDialog::serverDefinition() const
{
    return m_server;
}

void ManualMcpServerDialog::accept()
{
    bool ok = false;
    const int timeoutMs = m_timeoutEdit->text().trimmed().toInt(&ok);
    if (!ok || timeoutMs <= 0) {
        QMessageBox::warning(this, QStringLiteral("Invalid Timeout"), QStringLiteral("Timeout must be a positive integer."));
        return;
    }

    QString parseError;
    const QJsonObject env = parseJsonObjectText(m_envEdit->toPlainText(), &parseError);
    if (!parseError.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Invalid Env JSON"), parseError);
        return;
    }

    parseError.clear();
    const QJsonObject headers = parseJsonObjectText(m_headersEdit->toPlainText(), &parseError);
    if (!parseError.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Invalid Headers JSON"), parseError);
        return;
    }

    m_server = qtllm::tools::mcp::McpServerDefinition();
    m_server.serverId = normalizeId(m_idEdit->text());
    m_server.name = m_nameEdit->text().trimmed().isEmpty() ? m_server.serverId : m_nameEdit->text().trimmed();
    m_server.transport = m_transportCombo->currentText().trimmed().toLower();
    m_server.command = m_commandEdit->text().trimmed();
    m_server.args = parseArgs(m_argsEdit->text());
    m_server.url = m_urlEdit->text().trimmed();
    m_server.env = env;
    m_server.headers = headers;
    m_server.timeoutMs = timeoutMs;
    m_server.enabled = true;

    if (m_server.serverId.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Invalid Server ID"), QStringLiteral("Server ID is required."));
        return;
    }

    QDialog::accept();
}

QStringList ManualMcpServerDialog::parseArgs(const QString &text)
{
    QStringList items = text.split(',', Qt::SkipEmptyParts);
    for (QString &item : items) {
        item = item.trimmed();
    }
    items.removeAll(QString());
    return items;
}

QJsonObject ManualMcpServerDialog::parseJsonObjectText(const QString &text, QString *errorMessage)
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
