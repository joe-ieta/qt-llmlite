#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>

namespace pdftranslator::domain {

enum class DocumentTaskStage
{
    Idle,
    Queued,
    Extracting,
    DetectingLanguage,
    Translating,
    Assembling,
    Completed,
    Failed
};

QString documentTaskStageToString(DocumentTaskStage stage);

struct DocumentTranslationTask
{
    QString taskId;
    QString sourcePdfPath;
    QString sourceLanguage;
    QString targetLanguage;
    QString extractedTextPath;
    QString translatedMarkdownPath;
    QString translatedHtmlPath;
    QString manifestPath;
    QString lastError;
    int progress = 0;
    DocumentTaskStage stage = DocumentTaskStage::Idle;
    QDateTime createdAtUtc = QDateTime::currentDateTimeUtc();
    QDateTime updatedAtUtc = createdAtUtc;

    void transitionTo(DocumentTaskStage nextStage, const QString &errorMessage = QString());
    QJsonObject toJson() const;
    static DocumentTranslationTask fromJson(const QJsonObject &object);
};

} // namespace pdftranslator::domain
