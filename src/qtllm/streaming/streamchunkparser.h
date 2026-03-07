#pragma once

#include <QByteArray>
#include <QString>
#include <QStringList>

class StreamChunkParser
{
public:
    QStringList append(const QByteArray &chunk);
    QString takePendingLine();
    void clear();

private:
    QByteArray m_buffer;
};