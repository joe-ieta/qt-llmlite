#pragma once

#include <QByteArray>
#include <QStringList>

class StreamChunkParser
{
public:
    QStringList append(const QByteArray &chunk);
    void clear();

private:
    QByteArray m_buffer;
};
