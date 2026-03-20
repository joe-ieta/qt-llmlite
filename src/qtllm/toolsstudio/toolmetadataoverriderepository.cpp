#include "toolmetadataoverriderepository.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>

namespace qtllm::toolsstudio {

ToolMetadataOverrideRepository::ToolMetadataOverrideRepository(QString rootDirectory)
    : m_rootDirectory(std::move(rootDirectory))
{
}

bool ToolMetadataOverrideRepository::saveOverrides(const ToolMetadataOverridesSnapshot &snapshot,
                                                   QString *errorMessage) const
{
    if (!ensureRootDirectory(errorMessage)) {
        return false;
    }

    QSaveFile file(overridesPath());
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open metadata overrides for writing: ")
                            + file.errorString();
        }
        return false;
    }

    const QJsonDocument doc(snapshot.toJson());
    if (file.write(doc.toJson(QJsonDocument::Indented)) < 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to write metadata overrides: ") + file.errorString();
        }
        return false;
    }

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to commit metadata overrides: ") + file.errorString();
        }
        return false;
    }

    return true;
}

std::optional<ToolMetadataOverridesSnapshot> ToolMetadataOverrideRepository::loadOverrides(QString *errorMessage) const
{
    QFile file(overridesPath());
    if (!file.exists()) {
        return std::nullopt;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open metadata overrides for reading: ")
                            + file.errorString();
        }
        return std::nullopt;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Metadata overrides JSON parse error: ")
                            + parseError.errorString();
        }
        return std::nullopt;
    }

    return ToolMetadataOverridesSnapshot::fromJson(doc.object());
}

QString ToolMetadataOverrideRepository::overridesPath() const
{
    return QDir(m_rootDirectory).filePath(QStringLiteral("metadata_overrides.json"));
}

bool ToolMetadataOverrideRepository::ensureRootDirectory(QString *errorMessage) const
{
    QDir dir(m_rootDirectory);
    if (dir.exists() || dir.mkpath(QStringLiteral("."))) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Failed to create root directory: ") + m_rootDirectory;
    }
    return false;
}

} // namespace qtllm::toolsstudio
