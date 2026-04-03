#include "manifestrepository.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

namespace pdftranslator::storage {

bool ManifestRepository::save(const domain::DocumentTranslationTask &task, QString *errorMessage) const
{
    if (task.manifestPath.trimmed().isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Manifest path is empty");
        }
        return false;
    }

    const QFileInfo fileInfo(task.manifestPath);
    QDir dir = fileInfo.dir();
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to create manifest directory: ") + dir.absolutePath();
        }
        return false;
    }

    QFile file(task.manifestPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    const QJsonDocument document(task.toJson());
    file.write(document.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

std::optional<domain::DocumentTranslationTask> ManifestRepository::load(const QString &manifestPath,
                                                                        QString *errorMessage) const
{
    QFile file(manifestPath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return std::nullopt;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Manifest parse failed: %1").arg(parseError.errorString());
        }
        return std::nullopt;
    }

    return domain::DocumentTranslationTask::fromJson(document.object());
}

} // namespace pdftranslator::storage
