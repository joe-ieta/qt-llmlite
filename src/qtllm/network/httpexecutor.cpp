#include "httpexecutor.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

namespace qtllm {

HttpExecutor::HttpExecutor(QObject *parent)
    : QObject(parent)
    , m_networkAccessManager(new QNetworkAccessManager(this))
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, [this]() {
        if (!m_activeReply) {
            return;
        }

        m_timedOut = true;
        m_activeReply->abort();
    });
}

void HttpExecutor::post(const QNetworkRequest &request, const QByteArray &payload,
                        const HttpRequestOptions &options)
{
    cancel();

    m_request = request;
    m_payload = payload;
    m_options = options;

    if (m_options.timeoutMs <= 0) {
        m_options.timeoutMs = 60000;
    }
    if (m_options.retryDelayMs < 0) {
        m_options.retryDelayMs = 0;
    }
    if (m_options.maxRetries < 0) {
        m_options.maxRetries = 0;
    }

    m_buffer.clear();
    m_attempt = 0;
    m_timedOut = false;
    m_cancelRequested = false;

    startAttempt();
}

void HttpExecutor::cancel()
{
    m_timeoutTimer->stop();

    if (!m_activeReply) {
        return;
    }

    m_cancelRequested = true;
    m_activeReply->abort();
}

void HttpExecutor::startAttempt()
{
    if (m_activeReply) {
        m_activeReply->deleteLater();
        m_activeReply.clear();
    }

    m_buffer.clear();
    m_timedOut = false;
    ++m_attempt;

    m_activeReply = m_networkAccessManager->post(m_request, m_payload);
    m_timeoutTimer->start(m_options.timeoutMs);

    connect(m_activeReply, &QNetworkReply::readyRead, this, [this]() {
        if (!m_activeReply) {
            return;
        }

        const QByteArray chunk = m_activeReply->readAll();
        if (chunk.isEmpty()) {
            return;
        }

        m_buffer.append(chunk);
        emit dataReceived(chunk);
    });

    connect(m_activeReply, &QNetworkReply::finished, this, [this]() {
        if (!m_activeReply) {
            return;
        }

        m_timeoutTimer->stop();

        const auto error = m_activeReply->error();
        const QString errorString = m_activeReply->errorString();

        m_activeReply->deleteLater();
        m_activeReply.clear();

        if (m_cancelRequested) {
            m_cancelRequested = false;
            finishWithError(QStringLiteral("Request canceled"));
            return;
        }

        if (error != QNetworkReply::NoError) {
            if (canRetry()) {
                QTimer::singleShot(m_options.retryDelayMs, this, [this]() {
                    startAttempt();
                });
                return;
            }

            if (m_timedOut) {
                finishWithError(QStringLiteral("Request timeout"));
                return;
            }

            finishWithError(errorString);
            return;
        }

        emit requestFinished(m_buffer);
        m_buffer.clear();
    });
}

void HttpExecutor::finishWithError(const QString &message)
{
    m_buffer.clear();
    emit errorOccurred(message);
}

bool HttpExecutor::canRetry() const
{
    return m_attempt <= m_options.maxRetries;
}

} // namespace qtllm
