#pragma once

#include "filelogsink.h"

#include <QReadWriteLock>
#include <QVector>

#include <memory>

namespace qtllm::logging {

class QtLlmLogger
{
public:
    static QtLlmLogger &instance();

    void setMinimumLevel(LogLevel level);
    LogLevel minimumLevel() const;

    void addSink(const std::shared_ptr<ILogSink> &sink);
    void removeSink(const std::shared_ptr<ILogSink> &sink);
    void clearSinks();

    std::shared_ptr<FileLogSink> installFileSink(const FileLogSinkOptions &options);

    void log(LogLevel level,
             const QString &category,
             const QString &message,
             const LogContext &context = LogContext(),
             const QJsonObject &fields = QJsonObject());

    void trace(const QString &category,
               const QString &message,
               const LogContext &context = LogContext(),
               const QJsonObject &fields = QJsonObject());
    void debug(const QString &category,
               const QString &message,
               const LogContext &context = LogContext(),
               const QJsonObject &fields = QJsonObject());
    void info(const QString &category,
              const QString &message,
              const LogContext &context = LogContext(),
              const QJsonObject &fields = QJsonObject());
    void warn(const QString &category,
              const QString &message,
              const LogContext &context = LogContext(),
              const QJsonObject &fields = QJsonObject());
    void error(const QString &category,
               const QString &message,
               const LogContext &context = LogContext(),
               const QJsonObject &fields = QJsonObject());

private:
    QtLlmLogger() = default;

private:
    mutable QReadWriteLock m_lock;
    QVector<std::shared_ptr<ILogSink>> m_sinks;
    std::shared_ptr<FileLogSink> m_fileSink;
    LogLevel m_minimumLevel = LogLevel::Info;
};

} // namespace qtllm::logging
