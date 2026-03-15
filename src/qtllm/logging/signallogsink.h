#pragma once

#include "ilogsink.h"

#include <QObject>

namespace qtllm::logging {

class SignalLogSink : public QObject, public ILogSink
{
    Q_OBJECT
public:
    explicit SignalLogSink(QObject *parent = nullptr);
    void write(const LogEvent &event) override;

signals:
    void logEventReceived(const qtllm::logging::LogEvent &event);
};

} // namespace qtllm::logging
