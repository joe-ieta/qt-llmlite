#pragma once

#include "../domain/documenttranslationtask.h"

#include <QObject>
#include <QJsonArray>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <memory>

namespace pdftranslator::skills {
class SkillRegistry;
class ModelRouter;
}

namespace pdftranslator::pipeline {

struct BatchQueueEntry
{
    QString queueId;
    QString pdfPath;
    domain::DocumentTranslationTask task;
    QString extractedText;
    QString translatedText;
};

class BatchTranslationQueueController : public QObject
{
    Q_OBJECT
public:
    explicit BatchTranslationQueueController(const std::shared_ptr<skills::SkillRegistry> &skillRegistry,
                                            const std::shared_ptr<skills::ModelRouter> &modelRouter,
                                            QObject *parent = nullptr);
    ~BatchTranslationQueueController() override;

    void enqueueDocuments(const QStringList &pdfPaths);
    QVector<BatchQueueEntry> entries() const;
    bool isRunning() const;
    bool isPaused() const;
    int maxConcurrentJobs() const;
    int activeJobCount() const;
    void setPaused(bool paused);
    void setMaxConcurrentJobs(int value);
    bool retryEntry(const QString &queueId);
    QJsonArray toJson() const;
    void restoreFromJson(const QJsonArray &entriesJson);

signals:
    void queueChanged();
    void activeTaskChanged(const QString &queueId);
    void pausedChanged(bool paused);
    void activeJobsChanged(int activeCount, int maxConcurrentJobs);
    void taskProgressChanged(const QString &queueId, const pdftranslator::domain::DocumentTranslationTask &task);

private:
    void startQueuedJobs();

    struct ActiveRun
    {
        QObject *worker = nullptr;
        class QThread *thread = nullptr;
    };

private:
    std::shared_ptr<skills::SkillRegistry> m_skillRegistry;
    std::shared_ptr<skills::ModelRouter> m_modelRouter;
    QVector<BatchQueueEntry> m_entries;
    QString m_lastStartedQueueId;
    QHash<QString, ActiveRun> m_activeRuns;
    bool m_paused = false;
    int m_maxConcurrentJobs = 2;
};

} // namespace pdftranslator::pipeline

Q_DECLARE_METATYPE(pdftranslator::domain::DocumentTranslationTask)
Q_DECLARE_METATYPE(pdftranslator::pipeline::BatchQueueEntry)
