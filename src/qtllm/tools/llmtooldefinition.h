#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace qtllm::tools {

struct LlmToolDefinition
{
    QString toolId;
    QString invocationName;
    QString name;
    QString description;
    QJsonObject inputSchema;
    QStringList capabilityTags;
    QString category = QStringLiteral("custom");
    bool systemBuiltIn = false;
    bool removable = true;
    bool enabled = true;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("toolId"), toolId);
        root.insert(QStringLiteral("invocationName"), invocationName);
        root.insert(QStringLiteral("name"), name);
        root.insert(QStringLiteral("description"), description);
        root.insert(QStringLiteral("inputSchema"), inputSchema);
        root.insert(QStringLiteral("category"), category);
        root.insert(QStringLiteral("systemBuiltIn"), systemBuiltIn);
        root.insert(QStringLiteral("removable"), removable);
        root.insert(QStringLiteral("enabled"), enabled);

        QJsonArray tags;
        for (const QString &tag : capabilityTags) {
            tags.append(tag);
        }
        root.insert(QStringLiteral("capabilityTags"), tags);

        return root;
    }

    static LlmToolDefinition fromJson(const QJsonObject &root)
    {
        LlmToolDefinition tool;
        tool.toolId = root.value(QStringLiteral("toolId")).toString();
        tool.invocationName = root.value(QStringLiteral("invocationName")).toString();
        tool.name = root.value(QStringLiteral("name")).toString();
        tool.description = root.value(QStringLiteral("description")).toString();
        tool.inputSchema = root.value(QStringLiteral("inputSchema")).toObject();
        tool.category = root.value(QStringLiteral("category")).toString(tool.category);
        tool.systemBuiltIn = root.value(QStringLiteral("systemBuiltIn")).toBool(false);
        tool.removable = root.value(QStringLiteral("removable")).toBool(!tool.systemBuiltIn);
        tool.enabled = root.value(QStringLiteral("enabled")).toBool(true);

        if (tool.invocationName.trimmed().isEmpty()) {
            tool.invocationName = tool.name.trimmed().isEmpty() ? tool.toolId : tool.name;
        }
        if (tool.name.trimmed().isEmpty()) {
            tool.name = tool.invocationName.trimmed().isEmpty() ? tool.toolId : tool.invocationName;
        }

        const QJsonArray tags = root.value(QStringLiteral("capabilityTags")).toArray();
        for (const QJsonValue &value : tags) {
            const QString tag = value.toString();
            if (!tag.isEmpty()) {
                tool.capabilityTags.append(tag);
            }
        }

        return tool;
    }
};

} // namespace qtllm::tools
