#pragma once

#include <QObject>
#include <QPointer>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

class HttpExecutor : public QObject
{
    Q_OBJECT
public:
    explicit HttpExecutor(QObject *parent = nullptr);

    void post(const QNetworkRequest &request, const QByteArray &payload);

signals:
    void dataReceived(const QByteArray &chunk);
    void requestFinished(const QByteArray &data);
    void errorOccurred(const QString &message);

private:
    QNetworkAccessManager *m_networkAccessManager;
    QPointer<QNetworkReply> m_activeReply;
    QByteArray m_buffer;
};
