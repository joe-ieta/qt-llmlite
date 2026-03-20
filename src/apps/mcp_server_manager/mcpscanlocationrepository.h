#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>

#include <optional>

struct McpScanLocationConfig
{
    int schemaVersion = 1;
    QStringList builtInFilePatterns;
    QStringList customDirectories;

    QJsonObject toJson() const;
    static McpScanLocationConfig fromJson(const QJsonObject &root);
    static McpScanLocationConfig defaultConfig();
};

class McpScanLocationRepository
{
public:
    explicit McpScanLocationRepository(QString rootDirectory = QStringLiteral(".qtllm/mcp"));

    McpScanLocationConfig loadOrCreate(QString *errorMessage = nullptr) const;
    bool save(const McpScanLocationConfig &config, QString *errorMessage = nullptr) const;

private:
    QString configPath() const;
    bool ensureRootDirectory(QString *errorMessage = nullptr) const;

private:
    QString m_rootDirectory;
};
