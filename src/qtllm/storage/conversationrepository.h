#pragma once

#include "../chat/conversationsnapshot.h"

#include <QString>
#include <QStringList>

#include <optional>

namespace qtllm::storage {

class ConversationRepository
{
public:
    explicit ConversationRepository(QString rootDirectory = QStringLiteral(".qtllm"));

    bool saveSnapshot(const chat::ConversationSnapshot &snapshot, QString *errorMessage = nullptr) const;
    std::optional<chat::ConversationSnapshot> loadSnapshot(const QString &uid, QString *errorMessage = nullptr) const;

    QStringList listClientIds() const;

private:
    QString snapshotPath(const QString &uid) const;
    bool ensureRootDirectory(QString *errorMessage = nullptr) const;

private:
    QString m_rootDirectory;
};

} // namespace qtllm::storage
