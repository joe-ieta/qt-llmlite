#include "signallogsink.h"

#include <QMetaObject>

namespace qtllm::logging {

SignalLogSink::SignalLogSink(QObject *parent)
    : QObject(parent)
{
}

void SignalLogSink::write(const LogEvent &event)
{
    QMetaObject::invokeMethod(this,
                              [this, event]() { emit logEventReceived(event); },
                              Qt::QueuedConnection);
}

} // namespace qtllm::logging
