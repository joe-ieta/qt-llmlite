#include "filelogsink.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMutexLocker>
#include <QRegularExpression>

namespace qtllm::logging {

namespace {

QString dateStampUtc(const QDateTime &timestamp)
{
    return timestamp.toUTC().date().toString(QStringLiteral("yyyyMMdd"));
}

QString makeFileName(const QString &dateStamp, int index)
{
    return QStringLiteral("%1-%2.jsonl").arg(dateStamp, QString::number(index).rightJustified(4, QLatin1Char('0')));
}

} // namespace

FileLogSink::FileLogSink(FileLogSinkOptions options)
    : m_options(std::move(options))
{
}

FileLogSink::~FileLogSink() = default;

void FileLogSink::write(const LogEvent &event)
{
    const QByteArray line = QJsonDocument(event.toJson()).toJson(QJsonDocument::Compact) + '\n';
    const QString clientId = effectiveClientId(event.clientId);

    QMutexLocker locker(&m_mutex);
    ClientFileState &state = m_stateByClient[clientId];
    QString errorMessage;
    if (!ensureClientStateLocked(clientId, line.size(), &state, &errorMessage) || !state.file) {
        return;
    }

    if (state.file->write(line) == line.size()) {
        state.currentBytes += line.size();
        state.file->flush();
    }
}

void FileLogSink::setOptions(const FileLogSinkOptions &options)
{
    QMutexLocker locker(&m_mutex);
    m_options = options;
    m_stateByClient.clear();
}

FileLogSinkOptions FileLogSink::options() const
{
    QMutexLocker locker(&m_mutex);
    return m_options;
}

QString FileLogSink::logsRootPath() const
{
    QMutexLocker locker(&m_mutex);
    return logsRootPathUnlocked();
}

QString FileLogSink::resolvedWorkspaceRoot() const
{
    return m_options.workspaceRoot.trimmed().isEmpty() ? QDir::currentPath() : m_options.workspaceRoot.trimmed();
}

QString FileLogSink::logsRootPathUnlocked() const
{
    return QDir(resolvedWorkspaceRoot()).filePath(QStringLiteral(".qtllm/logs"));
}

QString FileLogSink::sanitizePathComponent(const QString &value) const
{
    QString sanitized = value.trimmed();
    if (sanitized.isEmpty()) {
        sanitized = m_options.systemClientId;
    }
    sanitized.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9._-]")), QStringLiteral("_"));
    if (sanitized.isEmpty()) {
        sanitized = m_options.systemClientId;
    }
    return sanitized;
}

QString FileLogSink::effectiveClientId(const QString &clientId) const
{
    return sanitizePathComponent(clientId.isEmpty() ? m_options.systemClientId : clientId);
}

bool FileLogSink::ensureLogsDirectoryLocked(QString *errorMessage) const
{
    QDir dir(logsRootPathUnlocked());
    if (dir.exists()) {
        return true;
    }

    if (dir.mkpath(QStringLiteral("."))) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Failed to create logs directory: ") + dir.path();
    }
    return false;
}

bool FileLogSink::ensureClientStateLocked(const QString &clientId,
                                          qint64 incomingBytes,
                                          ClientFileState *state,
                                          QString *errorMessage)
{
    if (!state) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Client file state is null");
        }
        return false;
    }

    if (!ensureLogsDirectoryLocked(errorMessage)) {
        return false;
    }

    const QString clientDirPath = QDir(logsRootPathUnlocked()).filePath(clientId);
    QDir clientDir(clientDirPath);
    if (!clientDir.exists() && !clientDir.mkpath(QStringLiteral("."))) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to create client log directory: ") + clientDirPath;
        }
        return false;
    }

    const QString currentDateStamp = dateStampUtc(QDateTime::currentDateTimeUtc());
    const bool needsRotate = !state->file
        || state->dateStamp != currentDateStamp
        || state->currentBytes + incomingBytes > qMax<qint64>(1024, m_options.maxBytesPerFile)
        || state->filePath.isEmpty();

    if (needsRotate) {
        return rotateLocked(clientId, incomingBytes, state, errorMessage);
    }

    return true;
}

bool FileLogSink::rotateLocked(const QString &clientId,
                               qint64 incomingBytes,
                               ClientFileState *state,
                               QString *errorMessage)
{
    const QString clientDirPath = QDir(logsRootPathUnlocked()).filePath(clientId);
    const QString currentDateStamp = dateStampUtc(QDateTime::currentDateTimeUtc());
    const bool sameDay = state->dateStamp == currentDateStamp;
    const bool sizeExceeded = state->file && state->currentBytes + incomingBytes > qMax<qint64>(1024, m_options.maxBytesPerFile);

    int nextIndex = 1;
    if (sameDay && sizeExceeded && state->fileIndex > 0) {
        nextIndex = state->fileIndex + 1;
    } else {
        nextIndex = nextFileIndexLocked(clientDirPath, currentDateStamp);
    }

    std::shared_ptr<QFile> file = std::make_shared<QFile>(QDir(clientDirPath).filePath(makeFileName(currentDateStamp, nextIndex)));
    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open log file: ") + file->errorString();
        }
        return false;
    }

    state->dateStamp = currentDateStamp;
    state->fileIndex = nextIndex;
    state->filePath = file->fileName();
    state->currentBytes = file->size();
    state->file = file;

    pruneLocked(clientDirPath);
    return true;
}

int FileLogSink::nextFileIndexLocked(const QString &clientDirPath, const QString &dateStamp) const
{
    QDir dir(clientDirPath);
    const QFileInfoList entries = dir.entryInfoList(QStringList() << (dateStamp + QStringLiteral("-*.jsonl")),
                                                    QDir::Files,
                                                    QDir::Name);
    int maxIndex = 0;
    for (const QFileInfo &entry : entries) {
        const QString base = entry.completeBaseName();
        const int dash = base.lastIndexOf(QLatin1Char('-'));
        if (dash < 0) {
            continue;
        }
        bool ok = false;
        const int index = base.mid(dash + 1).toInt(&ok);
        if (ok && index > maxIndex) {
            maxIndex = index;
        }
    }
    return maxIndex + 1;
}

void FileLogSink::pruneLocked(const QString &clientDirPath) const
{
    const int maxFiles = qMax(1, m_options.maxFilesPerClient);
    QDir dir(clientDirPath);
    const QFileInfoList entries = dir.entryInfoList(QStringList() << QStringLiteral("*.jsonl"),
                                                    QDir::Files,
                                                    QDir::Time | QDir::Reversed);
    const int removeCount = entries.size() - maxFiles;
    for (int i = 0; i < removeCount; ++i) {
        QFile::remove(entries.at(i).absoluteFilePath());
    }
}

} // namespace qtllm::logging

