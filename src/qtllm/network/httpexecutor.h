#pragma once

#include <QByteArray>
#include <QNetworkRequest>
#include <QObject>
#include <QPointer>

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

struct HttpRequestOptions
{
    int timeoutMs = 60000;
    int maxRetries = 0;
    int retryDelayMs = 400;
};

class HttpExecutor : public QObject
{
    Q_OBJECT
public:
    explicit HttpExecutor(QObject *parent = nullptr);

    void post(const QNetworkRequest &request, const QByteArray &payload,
              const HttpRequestOptions &options = HttpRequestOptions());
    void cancel();

signals:
    void dataReceived(const QByteArray &chunk);
    void requestFinished(const QByteArray &data);
    void errorOccurred(const QString &message);

private:
    void startAttempt();
    void finishWithError(const QString &message);
    bool canRetry() const;

private:
    QNetworkAccessManager *m_networkAccessManager;
    QPointer<QNetworkReply> m_activeReply;
    QTimer *m_timeoutTimer;

    QNetworkRequest m_request;
    QByteArray m_payload;
    HttpRequestOptions m_options;

    QByteArray m_buffer;
    int m_attempt = 0;
    bool m_timedOut = false;
    bool m_cancelRequested = false;
};