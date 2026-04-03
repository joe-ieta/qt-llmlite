#include "comparereaderwidget.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>
#include <QUrl>
#include <QVBoxLayout>

namespace pdftranslator::viewer {

CompareReaderWidget::CompareReaderWidget(QWidget *parent)
    : QWidget(parent)
{
    m_sourceLabel = new QLabel(QStringLiteral("Original"), this);
    m_targetLabel = new QLabel(QStringLiteral("Translated"), this);

    m_originalView = new QTextBrowser(this);
    m_originalView->setOpenExternalLinks(true);
    m_originalView->setPlaceholderText(QStringLiteral("Original extracted text will appear here."));

    m_translatedView = new QTextBrowser(this);
    m_translatedView->setOpenExternalLinks(true);
    m_translatedView->setPlaceholderText(QStringLiteral("Translated reading output will appear here."));

    m_openMarkdownButton = new QPushButton(QStringLiteral("Open Markdown"), this);
    m_openHtmlButton = new QPushButton(QStringLiteral("Open HTML"), this);

    connect(m_openMarkdownButton, &QPushButton::clicked, this, &CompareReaderWidget::openMarkdownOutput);
    connect(m_openHtmlButton, &QPushButton::clicked, this, &CompareReaderWidget::openHtmlOutput);

    auto *toolbarLayout = new QHBoxLayout();
    toolbarLayout->addWidget(m_sourceLabel);
    toolbarLayout->addStretch(1);
    toolbarLayout->addWidget(m_targetLabel);
    toolbarLayout->addStretch(1);
    toolbarLayout->addWidget(m_openMarkdownButton);
    toolbarLayout->addWidget(m_openHtmlButton);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(m_originalView);
    splitter->addWidget(m_translatedView);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(toolbarLayout);
    layout->addWidget(splitter, 1);
    setLayout(layout);

    updateButtons();
}

void CompareReaderWidget::setOriginalText(const QString &text)
{
    m_originalView->setPlainText(text);
}

void CompareReaderWidget::setTranslatedText(const QString &text)
{
    m_translatedView->setPlainText(text);
}

void CompareReaderWidget::setTranslatedHtml(const QString &html)
{
    if (html.trimmed().isEmpty()) {
        return;
    }
    m_translatedView->setHtml(html);
}

void CompareReaderWidget::setSourceLabel(const QString &text)
{
    m_sourceLabel->setText(text.trimmed().isEmpty() ? QStringLiteral("Original") : text);
}

void CompareReaderWidget::setTargetLabel(const QString &text)
{
    m_targetLabel->setText(text.trimmed().isEmpty() ? QStringLiteral("Translated") : text);
}

void CompareReaderWidget::setOutputPaths(const QString &markdownPath, const QString &htmlPath)
{
    m_markdownPath = markdownPath;
    m_htmlPath = htmlPath;
    updateButtons();
}

void CompareReaderWidget::openMarkdownOutput()
{
    if (!m_markdownPath.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_markdownPath));
    }
}

void CompareReaderWidget::openHtmlOutput()
{
    if (!m_htmlPath.isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_htmlPath));
    }
}

void CompareReaderWidget::updateButtons()
{
    m_openMarkdownButton->setEnabled(QFileInfo::exists(m_markdownPath));
    m_openHtmlButton->setEnabled(QFileInfo::exists(m_htmlPath));
}

} // namespace pdftranslator::viewer
