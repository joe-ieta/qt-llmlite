#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QMetaType>

namespace qtllm::logging {

enum class LogLevel
{
    Trace = 0,
    Debug,
    Info,
    Warn,
    Error
};

struct LogContext
{
    QString clientId;
    QString sessionId;
    QString requestId;
    QString traceId;
};

struct LogEvent
{
    QDateTime timestampUtc = QDateTime::currentDateTimeUtc();
    LogLevel level = LogLevel::Info;
    QString category;
    QString message;
    QString clientId;
    QString sessionId;
    QString requestId;
    QString traceId;
    QString threadId;
    QJsonObject fields;

    QJsonObject toJson() const;
};

QString logLevelToString(LogLevel level);
LogEvent makeLogEvent(LogLevel level,
                      const QString &category,
                      const QString &message,
                      const LogContext &context = LogContext(),
                      const QJsonObject &fields = QJsonObject());

} // namespace qtllm::logging

Q_DECLARE_METATYPE(qtllm::logging::LogLevel)
Q_DECLARE_METATYPE(qtllm::logging::LogEvent)
