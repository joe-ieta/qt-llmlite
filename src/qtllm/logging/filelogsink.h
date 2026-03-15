#pragma once

#include "ilogsink.h"

#include <QHash>
#include <QMutex>
#include <QString>

#include <memory>

class QFile;

namespace qtllm::logging {

struct FileLogSinkOptions
{
    QString workspaceRoot;
    qint64 maxBytesPerFile = 5 * 1024 * 1024;
    int maxFilesPerClient = 20;
    QString systemClientId = QStringLiteral("_system");
};

class FileLogSink : public ILogSink
{
public:
    explicit FileLogSink(FileLogSinkOptions options = FileLogSinkOptions());
    ~FileLogSink() override;

    void write(const LogEvent &event) override;

    void setOptions(const FileLogSinkOptions &options);
    FileLogSinkOptions options() const;
    QString logsRootPath() const;

private:
    struct ClientFileState
    {
        QString dateStamp;
        int fileIndex = 1;
        QString filePath;
        qint64 currentBytes = 0;
        std::shared_ptr<QFile> file;
    };

    QString resolvedWorkspaceRoot() const;
    QString logsRootPathUnlocked() const;
    QString sanitizePathComponent(const QString &value) const;
    QString effectiveClientId(const QString &clientId) const;
    bool ensureLogsDirectoryLocked(QString *errorMessage = nullptr) const;
    bool ensureClientStateLocked(const QString &clientId,
                                 qint64 incomingBytes,
                                 ClientFileState *state,
                                 QString *errorMessage = nullptr);
    bool rotateLocked(const QString &clientId,
                      qint64 incomingBytes,
                      ClientFileState *state,
                      QString *errorMessage = nullptr);
    int nextFileIndexLocked(const QString &clientDirPath, const QString &dateStamp) const;
    void pruneLocked(const QString &clientDirPath) const;

private:
    mutable QMutex m_mutex;
    FileLogSinkOptions m_options;
    mutable QHash<QString, ClientFileState> m_stateByClient;
};

} // namespace qtllm::logging
