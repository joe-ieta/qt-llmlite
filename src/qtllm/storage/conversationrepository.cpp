#include "conversationrepository.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>

namespace qtllm::storage {

ConversationRepository::ConversationRepository(QString rootDirectory)
    : m_rootDirectory(std::move(rootDirectory))
{
}

bool ConversationRepository::saveSnapshot(const chat::ConversationSnapshot &snapshot, QString *errorMessage) const
{
    if (snapshot.uid.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Snapshot uid is empty");
        }
        return false;
    }

    if (!ensureRootDirectory(errorMessage)) {
        return false;
    }

    QSaveFile file(snapshotPath(snapshot.uid));
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open snapshot for writing: ") + file.errorString();
        }
        return false;
    }

    const QJsonDocument doc(snapshot.toJson());
    if (file.write(doc.toJson(QJsonDocument::Indented)) < 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to write snapshot: ") + file.errorString();
        }
        return false;
    }

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to commit snapshot: ") + file.errorString();
        }
        return false;
    }

    return true;
}

std::optional<chat::ConversationSnapshot> ConversationRepository::loadSnapshot(const QString &uid, QString *errorMessage) const
{
    const QString resolvedUid = uid.trimmed();
    if (resolvedUid.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Snapshot uid is empty");
        }
        return std::nullopt;
    }

    QFile file(snapshotPath(resolvedUid));
    if (!file.exists()) {
        return std::nullopt;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open snapshot for reading: ") + file.errorString();
        }
        return std::nullopt;
    }

    const QByteArray payload = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Snapshot JSON parse error: ") + parseError.errorString();
        }
        return std::nullopt;
    }

    return chat::ConversationSnapshot::fromJson(doc.object());
}

QStringList ConversationRepository::listClientIds() const
{
    QDir dir(m_rootDirectory);
    if (!dir.exists()) {
        return {};
    }

    QStringList ids;
    const QFileInfoList files = dir.entryInfoList(QStringList() << QStringLiteral("*.json"), QDir::Files);
    for (const QFileInfo &fileInfo : files) {
        ids.append(fileInfo.completeBaseName());
    }
    return ids;
}

QString ConversationRepository::snapshotPath(const QString &uid) const
{
    return QDir(m_rootDirectory).filePath(uid + QStringLiteral(".json"));
}

bool ConversationRepository::ensureRootDirectory(QString *errorMessage) const
{
    QDir dir(m_rootDirectory);
    if (dir.exists()) {
        return true;
    }

    if (dir.mkpath(QStringLiteral("."))) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Failed to create repository directory: ") + m_rootDirectory;
    }
    return false;
}

} // namespace qtllm::storage
