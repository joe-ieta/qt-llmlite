#include "toolimportpreviewdialog.h"

#include <QDialogButtonBox>
#include <QJsonDocument>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

ToolImportPreviewDialog::ToolImportPreviewDialog(const qtllm::toolsstudio::ToolMergePreview &preview, QWidget *parent)
    : QDialog(parent)
    , m_summaryView(new QTextEdit(this))
{
    setWindowTitle(QStringLiteral("Import Preview"));
    resize(860, 640);

    m_summaryView->setReadOnly(true);

    QStringList sections;
    sections.append(QStringLiteral("Summary\n%1").arg(preview.summary));

    if (!preview.decisions.isEmpty()) {
        QStringList lines;
        for (const qtllm::toolsstudio::ToolMergeDecision &decision : preview.decisions) {
            lines.append(QStringLiteral("- [%1] %2 (%3)")
                             .arg(decision.action,
                                  decision.toolId.isEmpty() ? QStringLiteral("<workspace>") : decision.toolId,
                                  decision.reason));
        }
        sections.append(QStringLiteral("Decisions\n%1").arg(lines.join(QStringLiteral("\n"))));
    }

    if (!preview.issues.isEmpty()) {
        QStringList lines;
        for (const qtllm::toolsstudio::ToolMergeIssue &issue : preview.issues) {
            lines.append(QStringLiteral("- [%1/%2] %3: %4")
                             .arg(issue.severity,
                                  issue.issueType,
                                  issue.toolId.isEmpty() ? QStringLiteral("<workspace>") : issue.toolId,
                                  issue.message));
        }
        sections.append(QStringLiteral("Issues\n%1").arg(lines.join(QStringLiteral("\n"))));
    }

    if (!preview.toolsToInsert.isEmpty()) {
        QStringList lines;
        for (const qtllm::tools::LlmToolDefinition &tool : preview.toolsToInsert) {
            lines.append(QStringLiteral("- %1 | %2").arg(tool.toolId, tool.name));
        }
        sections.append(QStringLiteral("Tools To Insert\n%1").arg(lines.join(QStringLiteral("\n"))));
    }

    sections.append(QStringLiteral("Merged Workspace Snapshot\n%1")
                        .arg(QString::fromUtf8(QJsonDocument(preview.mergedWorkspace.toJson()).toJson(QJsonDocument::Indented))));
    m_summaryView->setPlainText(sections.join(QStringLiteral("\n\n")));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("Apply Import"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("Cancel"));
    connect(buttons, &QDialogButtonBox::accepted, this, &ToolImportPreviewDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &ToolImportPreviewDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_summaryView, 1);
    layout->addWidget(buttons);
}
