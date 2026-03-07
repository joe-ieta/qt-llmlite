#include "streamchunkparser.h"

namespace qtllm {

QStringList StreamChunkParser::append(const QByteArray &chunk)
{
    m_buffer.append(chunk);

    QStringList completeLines;
    int index = -1;
    while ((index = m_buffer.indexOf('\n')) >= 0) {
        const QByteArray line = m_buffer.left(index);
        m_buffer.remove(0, index + 1);
        completeLines.append(QString::fromUtf8(line));
    }

    return completeLines;
}

QString StreamChunkParser::takePendingLine()
{
    if (m_buffer.isEmpty()) {
        return QString();
    }

    const QString pending = QString::fromUtf8(m_buffer);
    m_buffer.clear();
    return pending;
}

void StreamChunkParser::clear()
{
    m_buffer.clear();
}

} // namespace qtllm
