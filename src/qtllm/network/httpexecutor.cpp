#include "httpexecutor.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

HttpExecutor::HttpExecutor(QObject *parent)
    : QObject(parent)
    , m_networkAccessManager(new QNetworkAccessManager(this))
{
}

void HttpExecutor::post(const QNetworkRequest &request, const QByteArray &payload)
{
    m_buffer.clear();

    if (m_activeReply) {
        m_activeReply->deleteLater();
        m_activeReply.clear();
    }

    m_activeReply = m_networkAccessManager->post(request, payload);

    connect(m_activeReply, &QNetworkReply::readyRead, this, [this]() {
        if (!m_activeReply) {
            return;
        }
        const QByteArray chunk = m_activeReply->readAll();
        if (!chunk.isEmpty()) {
            m_buffer.append(chunk);
            emit dataReceived(chunk);
        }
    });

    connect(m_activeReply, &QNetworkReply::finished, this, [this]() {
        if (!m_activeReply) {
            return;
        }

        const auto error = m_activeReply->error();
        const QString errorString = m_activeReply->errorString();
        m_activeReply->deleteLater();
        m_activeReply.clear();

        if (error != QNetworkReply::NoError) {
            emit errorOccurred(errorString);
            return;
        }

        emit requestFinished(m_buffer);
    });
}
