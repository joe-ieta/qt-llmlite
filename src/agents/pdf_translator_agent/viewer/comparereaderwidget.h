#pragma once

#include <QWidget>

class QLabel;
class QTextBrowser;
class QPushButton;
class QUrl;

namespace pdftranslator::viewer {

class CompareReaderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CompareReaderWidget(QWidget *parent = nullptr);

    void setOriginalText(const QString &text);
    void setTranslatedText(const QString &text);
    void setTranslatedHtml(const QString &html);
    void setSourceLabel(const QString &text);
    void setTargetLabel(const QString &text);
    void setOutputPaths(const QString &markdownPath, const QString &htmlPath);

private slots:
    void openMarkdownOutput();
    void openHtmlOutput();

private:
    void updateButtons();

private:
    QLabel *m_sourceLabel = nullptr;
    QLabel *m_targetLabel = nullptr;
    QTextBrowser *m_originalView = nullptr;
    QTextBrowser *m_translatedView = nullptr;
    QPushButton *m_openMarkdownButton = nullptr;
    QPushButton *m_openHtmlButton = nullptr;
    QString m_markdownPath;
    QString m_htmlPath;
};

} // namespace pdftranslator::viewer
