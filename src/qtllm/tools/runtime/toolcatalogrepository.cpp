#include "toolcatalogrepository.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>

namespace qtllm::tools::runtime {

ToolCatalogRepository::ToolCatalogRepository(QString rootDirectory)
    : m_rootDirectory(std::move(rootDirectory))
{
}

bool ToolCatalogRepository::saveCatalog(const ToolCatalogSnapshot &snapshot, QString *errorMessage) const
{
    if (!ensureRootDirectory(errorMessage)) {
        return false;
    }

    QSaveFile file(catalogPath());
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open tool catalog for writing: ") + file.errorString();
        }
        return false;
    }

    QJsonObject root;
    root.insert(QStringLiteral("schemaVersion"), snapshot.schemaVersion);
    root.insert(QStringLiteral("updatedAt"), snapshot.updatedAt.toString(Qt::ISODateWithMs));

    QJsonArray toolsArray;
    for (const tools::LlmToolDefinition &tool : snapshot.tools) {
        toolsArray.append(tool.toJson());
    }
    root.insert(QStringLiteral("tools"), toolsArray);

    const QJsonDocument doc(root);
    if (file.write(doc.toJson(QJsonDocument::Indented)) < 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to write tool catalog: ") + file.errorString();
        }
        return false;
    }

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to commit tool catalog: ") + file.errorString();
        }
        return false;
    }

    return true;
}

std::optional<ToolCatalogSnapshot> ToolCatalogRepository::loadCatalog(QString *errorMessage) const
{
    QFile file(catalogPath());
    if (!file.exists()) {
        return std::nullopt;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open tool catalog for reading: ") + file.errorString();
        }
        return std::nullopt;
    }

    const QByteArray payload = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Tool catalog JSON parse error: ") + parseError.errorString();
        }
        return std::nullopt;
    }

    ToolCatalogSnapshot snapshot;
    const QJsonObject root = doc.object();
    snapshot.schemaVersion = root.value(QStringLiteral("schemaVersion")).toInt(snapshot.schemaVersion);

    const QDateTime updatedAt = QDateTime::fromString(root.value(QStringLiteral("updatedAt")).toString(),
                                                       Qt::ISODateWithMs);
    if (updatedAt.isValid()) {
        snapshot.updatedAt = updatedAt;
    }

    const QJsonArray toolsArray = root.value(QStringLiteral("tools")).toArray();
    for (const QJsonValue &value : toolsArray) {
        const tools::LlmToolDefinition tool = tools::LlmToolDefinition::fromJson(value.toObject());
        if (!tool.toolId.isEmpty()) {
            snapshot.tools.append(tool);
        }
    }

    return snapshot;
}

QString ToolCatalogRepository::catalogPath() const
{
    return QDir(m_rootDirectory).filePath(QStringLiteral("catalog.json"));
}

bool ToolCatalogRepository::ensureRootDirectory(QString *errorMessage) const
{
    QDir dir(m_rootDirectory);
    if (dir.exists()) {
        return true;
    }

    if (dir.mkpath(QStringLiteral("."))) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Failed to create tool catalog directory: ") + m_rootDirectory;
    }
    return false;
}

} // namespace qtllm::tools::runtime
