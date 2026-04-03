#include "documenttranslationtask.h"

namespace pdftranslator::domain {

QString documentTaskStageToString(DocumentTaskStage stage)
{
    switch (stage) {
    case DocumentTaskStage::Idle:
        return QStringLiteral("idle");
    case DocumentTaskStage::Queued:
        return QStringLiteral("queued");
    case DocumentTaskStage::Extracting:
        return QStringLiteral("extracting");
    case DocumentTaskStage::DetectingLanguage:
        return QStringLiteral("detecting-language");
    case DocumentTaskStage::Translating:
        return QStringLiteral("translating");
    case DocumentTaskStage::Assembling:
        return QStringLiteral("assembling");
    case DocumentTaskStage::Completed:
        return QStringLiteral("completed");
    case DocumentTaskStage::Failed:
        return QStringLiteral("failed");
    }
    return QStringLiteral("idle");
}

namespace {

DocumentTaskStage documentTaskStageFromString(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("queued")) {
        return DocumentTaskStage::Queued;
    }
    if (normalized == QStringLiteral("extracting")) {
        return DocumentTaskStage::Extracting;
    }
    if (normalized == QStringLiteral("detecting-language")) {
        return DocumentTaskStage::DetectingLanguage;
    }
    if (normalized == QStringLiteral("translating")) {
        return DocumentTaskStage::Translating;
    }
    if (normalized == QStringLiteral("assembling")) {
        return DocumentTaskStage::Assembling;
    }
    if (normalized == QStringLiteral("completed")) {
        return DocumentTaskStage::Completed;
    }
    if (normalized == QStringLiteral("failed")) {
        return DocumentTaskStage::Failed;
    }
    return DocumentTaskStage::Idle;
}

} // namespace

void DocumentTranslationTask::transitionTo(DocumentTaskStage nextStage, const QString &errorMessage)
{
    stage = nextStage;
    updatedAtUtc = QDateTime::currentDateTimeUtc();
    if (nextStage == DocumentTaskStage::Failed) {
        lastError = errorMessage.trimmed();
    } else if (!errorMessage.trimmed().isEmpty()) {
        lastError = errorMessage.trimmed();
    }
}

QJsonObject DocumentTranslationTask::toJson() const
{
    QJsonObject object;
    object.insert(QStringLiteral("taskId"), taskId);
    object.insert(QStringLiteral("sourcePdfPath"), sourcePdfPath);
    object.insert(QStringLiteral("sourceLanguage"), sourceLanguage);
    object.insert(QStringLiteral("targetLanguage"), targetLanguage);
    object.insert(QStringLiteral("extractedTextPath"), extractedTextPath);
    object.insert(QStringLiteral("translatedMarkdownPath"), translatedMarkdownPath);
    object.insert(QStringLiteral("translatedHtmlPath"), translatedHtmlPath);
    object.insert(QStringLiteral("manifestPath"), manifestPath);
    object.insert(QStringLiteral("lastError"), lastError);
    object.insert(QStringLiteral("progress"), progress);
    object.insert(QStringLiteral("stage"), documentTaskStageToString(stage));
    object.insert(QStringLiteral("createdAtUtc"), createdAtUtc.toString(Qt::ISODateWithMs));
    object.insert(QStringLiteral("updatedAtUtc"), updatedAtUtc.toString(Qt::ISODateWithMs));
    return object;
}

DocumentTranslationTask DocumentTranslationTask::fromJson(const QJsonObject &object)
{
    DocumentTranslationTask task;
    task.taskId = object.value(QStringLiteral("taskId")).toString();
    task.sourcePdfPath = object.value(QStringLiteral("sourcePdfPath")).toString();
    task.sourceLanguage = object.value(QStringLiteral("sourceLanguage")).toString();
    task.targetLanguage = object.value(QStringLiteral("targetLanguage")).toString();
    task.extractedTextPath = object.value(QStringLiteral("extractedTextPath")).toString();
    task.translatedMarkdownPath = object.value(QStringLiteral("translatedMarkdownPath")).toString();
    task.translatedHtmlPath = object.value(QStringLiteral("translatedHtmlPath")).toString();
    task.manifestPath = object.value(QStringLiteral("manifestPath")).toString();
    task.lastError = object.value(QStringLiteral("lastError")).toString();
    task.progress = object.value(QStringLiteral("progress")).toInt();
    task.stage = documentTaskStageFromString(object.value(QStringLiteral("stage")).toString());
    task.createdAtUtc = QDateTime::fromString(object.value(QStringLiteral("createdAtUtc")).toString(), Qt::ISODateWithMs);
    task.updatedAtUtc = QDateTime::fromString(object.value(QStringLiteral("updatedAtUtc")).toString(), Qt::ISODateWithMs);
    if (!task.createdAtUtc.isValid()) {
        task.createdAtUtc = QDateTime::currentDateTimeUtc();
    }
    if (!task.updatedAtUtc.isValid()) {
        task.updatedAtUtc = task.createdAtUtc;
    }
    return task;
}

} // namespace pdftranslator::domain
