#include "logtypes.h"

#include <QJsonValue>
#include <QThread>

namespace qtllm::logging {

namespace {

QString currentThreadIdText()
{
    return QStringLiteral("0x%1").arg(reinterpret_cast<quintptr>(QThread::currentThreadId()), 0, 16);
}

} // namespace

QJsonObject LogEvent::toJson() const
{
    QJsonObject root;
    root.insert(QStringLiteral("timestampUtc"), timestampUtc.toString(Qt::ISODateWithMs));
    root.insert(QStringLiteral("level"), logLevelToString(level));
    root.insert(QStringLiteral("category"), category);
    root.insert(QStringLiteral("message"), message);
    if (!clientId.isEmpty()) {
        root.insert(QStringLiteral("clientId"), clientId);
    }
    if (!sessionId.isEmpty()) {
        root.insert(QStringLiteral("sessionId"), sessionId);
    }
    if (!requestId.isEmpty()) {
        root.insert(QStringLiteral("requestId"), requestId);
    }
    if (!traceId.isEmpty()) {
        root.insert(QStringLiteral("traceId"), traceId);
    }
    if (!threadId.isEmpty()) {
        root.insert(QStringLiteral("threadId"), threadId);
    }
    if (!fields.isEmpty()) {
        root.insert(QStringLiteral("fields"), fields);
    }
    return root;
}

QString logLevelToString(LogLevel level)
{
    switch (level) {
    case LogLevel::Trace:
        return QStringLiteral("trace");
    case LogLevel::Debug:
        return QStringLiteral("debug");
    case LogLevel::Info:
        return QStringLiteral("info");
    case LogLevel::Warn:
        return QStringLiteral("warn");
    case LogLevel::Error:
        return QStringLiteral("error");
    }

    return QStringLiteral("info");
}

LogEvent makeLogEvent(LogLevel level,
                      const QString &category,
                      const QString &message,
                      const LogContext &context,
                      const QJsonObject &fields)
{
    LogEvent event;
    event.level = level;
    event.category = category.trimmed();
    event.message = message;
    event.clientId = context.clientId.trimmed();
    event.sessionId = context.sessionId.trimmed();
    event.requestId = context.requestId.trimmed();
    event.traceId = context.traceId.trimmed();
    event.threadId = currentThreadIdText();
    event.fields = fields;
    return event;
}

} // namespace qtllm::logging
