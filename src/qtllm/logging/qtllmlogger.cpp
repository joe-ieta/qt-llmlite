#include "qtllmlogger.h"

#include <QReadLocker>
#include <QWriteLocker>

namespace qtllm::logging {

QtLlmLogger &QtLlmLogger::instance()
{
    static QtLlmLogger logger;
    return logger;
}

void QtLlmLogger::setMinimumLevel(LogLevel level)
{
    QWriteLocker locker(&m_lock);
    m_minimumLevel = level;
}

LogLevel QtLlmLogger::minimumLevel() const
{
    QReadLocker locker(&m_lock);
    return m_minimumLevel;
}

void QtLlmLogger::addSink(const std::shared_ptr<ILogSink> &sink)
{
    if (!sink) {
        return;
    }

    QWriteLocker locker(&m_lock);
    for (const std::shared_ptr<ILogSink> &existing : m_sinks) {
        if (existing == sink) {
            return;
        }
    }
    m_sinks.append(sink);
}

void QtLlmLogger::removeSink(const std::shared_ptr<ILogSink> &sink)
{
    QWriteLocker locker(&m_lock);
    for (int i = m_sinks.size() - 1; i >= 0; --i) {
        if (m_sinks.at(i) == sink) {
            m_sinks.remove(i);
        }
    }
    if (m_fileSink == sink) {
        m_fileSink.reset();
    }
}

void QtLlmLogger::clearSinks()
{
    QWriteLocker locker(&m_lock);
    m_sinks.clear();
    m_fileSink.reset();
}

std::shared_ptr<FileLogSink> QtLlmLogger::installFileSink(const FileLogSinkOptions &options)
{
    QWriteLocker locker(&m_lock);
    if (!m_fileSink) {
        m_fileSink = std::make_shared<FileLogSink>(options);
        m_sinks.append(m_fileSink);
    } else {
        m_fileSink->setOptions(options);
    }
    return m_fileSink;
}

void QtLlmLogger::log(LogLevel level,
                      const QString &category,
                      const QString &message,
                      const LogContext &context,
                      const QJsonObject &fields)
{
    QVector<std::shared_ptr<ILogSink>> sinks;
    {
        QReadLocker locker(&m_lock);
        if (static_cast<int>(level) < static_cast<int>(m_minimumLevel)) {
            return;
        }
        sinks = m_sinks;
    }

    if (sinks.isEmpty()) {
        return;
    }

    const LogEvent event = makeLogEvent(level, category, message, context, fields);
    for (const std::shared_ptr<ILogSink> &sink : sinks) {
        if (sink) {
            sink->write(event);
        }
    }
}

void QtLlmLogger::trace(const QString &category,
                        const QString &message,
                        const LogContext &context,
                        const QJsonObject &fields)
{
    log(LogLevel::Trace, category, message, context, fields);
}

void QtLlmLogger::debug(const QString &category,
                        const QString &message,
                        const LogContext &context,
                        const QJsonObject &fields)
{
    log(LogLevel::Debug, category, message, context, fields);
}

void QtLlmLogger::info(const QString &category,
                       const QString &message,
                       const LogContext &context,
                       const QJsonObject &fields)
{
    log(LogLevel::Info, category, message, context, fields);
}

void QtLlmLogger::warn(const QString &category,
                       const QString &message,
                       const LogContext &context,
                       const QJsonObject &fields)
{
    log(LogLevel::Warn, category, message, context, fields);
}

void QtLlmLogger::error(const QString &category,
                        const QString &message,
                        const LogContext &context,
                        const QJsonObject &fields)
{
    log(LogLevel::Error, category, message, context, fields);
}

} // namespace qtllm::logging
