#pragma once

#include "../tools/llmtooldefinition.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QStringList>
#include <QVector>

namespace qtllm::toolsstudio {

namespace detail {

inline QJsonArray toJsonArray(const QStringList &values)
{
    QJsonArray array;
    for (const QString &value : values) {
        const QString trimmed = value.trimmed();
        if (!trimmed.isEmpty()) {
            array.append(trimmed);
        }
    }
    return array;
}

inline QStringList fromJsonArray(const QJsonValue &value)
{
    QStringList out;
    const QJsonArray array = value.toArray();
    for (const QJsonValue &entry : array) {
        const QString item = entry.toString().trimmed();
        if (!item.isEmpty()) {
            out.append(item);
        }
    }
    return out;
}

} // namespace detail

struct ToolStudioEditableFields
{
    QString name;
    QString description;
    QJsonObject inputSchema;
    QStringList capabilityTags;
    bool enabled = true;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("name"), name);
        root.insert(QStringLiteral("description"), description);
        root.insert(QStringLiteral("inputSchema"), inputSchema);
        root.insert(QStringLiteral("capabilityTags"), detail::toJsonArray(capabilityTags));
        root.insert(QStringLiteral("enabled"), enabled);
        return root;
    }

    static ToolStudioEditableFields fromJson(const QJsonObject &root)
    {
        ToolStudioEditableFields fields;
        fields.name = root.value(QStringLiteral("name")).toString();
        fields.description = root.value(QStringLiteral("description")).toString();
        fields.inputSchema = root.value(QStringLiteral("inputSchema")).toObject();
        fields.capabilityTags = detail::fromJsonArray(root.value(QStringLiteral("capabilityTags")));
        fields.enabled = root.value(QStringLiteral("enabled")).toBool(true);
        return fields;
    }
};

struct ToolMetadataOverride
{
    QString toolId;
    ToolStudioEditableFields fields;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("toolId"), toolId);
        root.insert(QStringLiteral("fields"), fields.toJson());
        return root;
    }

    static ToolMetadataOverride fromJson(const QJsonObject &root)
    {
        ToolMetadataOverride overrideEntry;
        overrideEntry.toolId = root.value(QStringLiteral("toolId")).toString();
        overrideEntry.fields = ToolStudioEditableFields::fromJson(root.value(QStringLiteral("fields")).toObject());
        return overrideEntry;
    }
};

struct ToolMetadataOverridesSnapshot
{
    int schemaVersion = 1;
    QVector<ToolMetadataOverride> overrides;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("schemaVersion"), schemaVersion);

        QJsonArray items;
        for (const ToolMetadataOverride &entry : overrides) {
            items.append(entry.toJson());
        }
        root.insert(QStringLiteral("overrides"), items);
        return root;
    }

    static ToolMetadataOverridesSnapshot fromJson(const QJsonObject &root)
    {
        ToolMetadataOverridesSnapshot snapshot;
        snapshot.schemaVersion = root.value(QStringLiteral("schemaVersion")).toInt(snapshot.schemaVersion);

        const QJsonArray items = root.value(QStringLiteral("overrides")).toArray();
        for (const QJsonValue &value : items) {
            if (value.isObject()) {
                snapshot.overrides.append(ToolMetadataOverride::fromJson(value.toObject()));
            }
        }
        return snapshot;
    }
};

struct ToolCategoryNode
{
    QString nodeId;
    QString parentNodeId;
    QString name;
    QString description;
    int order = 0;
    bool expanded = true;
    QString color;
    QString iconKey;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("nodeId"), nodeId);
        root.insert(QStringLiteral("parentNodeId"), parentNodeId);
        root.insert(QStringLiteral("name"), name);
        root.insert(QStringLiteral("description"), description);
        root.insert(QStringLiteral("order"), order);
        root.insert(QStringLiteral("expanded"), expanded);
        root.insert(QStringLiteral("color"), color);
        root.insert(QStringLiteral("iconKey"), iconKey);
        return root;
    }

    static ToolCategoryNode fromJson(const QJsonObject &root)
    {
        ToolCategoryNode node;
        node.nodeId = root.value(QStringLiteral("nodeId")).toString();
        node.parentNodeId = root.value(QStringLiteral("parentNodeId")).toString();
        node.name = root.value(QStringLiteral("name")).toString();
        node.description = root.value(QStringLiteral("description")).toString();
        node.order = root.value(QStringLiteral("order")).toInt(0);
        node.expanded = root.value(QStringLiteral("expanded")).toBool(true);
        node.color = root.value(QStringLiteral("color")).toString();
        node.iconKey = root.value(QStringLiteral("iconKey")).toString();
        return node;
    }
};

struct ToolPlacement
{
    QString placementId;
    QString nodeId;
    QString toolId;
    int order = 0;
    QStringList localTags;
    QString note;
    bool pinned = false;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("placementId"), placementId);
        root.insert(QStringLiteral("nodeId"), nodeId);
        root.insert(QStringLiteral("toolId"), toolId);
        root.insert(QStringLiteral("order"), order);
        root.insert(QStringLiteral("localTags"), detail::toJsonArray(localTags));
        root.insert(QStringLiteral("note"), note);
        root.insert(QStringLiteral("pinned"), pinned);
        return root;
    }

    static ToolPlacement fromJson(const QJsonObject &root)
    {
        ToolPlacement placement;
        placement.placementId = root.value(QStringLiteral("placementId")).toString();
        placement.nodeId = root.value(QStringLiteral("nodeId")).toString();
        placement.toolId = root.value(QStringLiteral("toolId")).toString();
        placement.order = root.value(QStringLiteral("order")).toInt(0);
        placement.localTags = detail::fromJsonArray(root.value(QStringLiteral("localTags")));
        placement.note = root.value(QStringLiteral("note")).toString();
        placement.pinned = root.value(QStringLiteral("pinned")).toBool(false);
        return placement;
    }
};

struct ToolWorkspaceSnapshot
{
    int schemaVersion = 1;
    QString workspaceId;
    QString name;
    QString description;
    QDateTime updatedAt = QDateTime::currentDateTimeUtc();
    QString rootNodeId = QStringLiteral("root");
    QVector<ToolCategoryNode> nodes;
    QVector<ToolPlacement> placements;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("schemaVersion"), schemaVersion);
        root.insert(QStringLiteral("workspaceId"), workspaceId);
        root.insert(QStringLiteral("name"), name);
        root.insert(QStringLiteral("description"), description);
        root.insert(QStringLiteral("updatedAt"), updatedAt.toString(Qt::ISODateWithMs));
        root.insert(QStringLiteral("rootNodeId"), rootNodeId);

        QJsonArray nodesArray;
        for (const ToolCategoryNode &node : nodes) {
            nodesArray.append(node.toJson());
        }
        root.insert(QStringLiteral("nodes"), nodesArray);

        QJsonArray placementArray;
        for (const ToolPlacement &placement : placements) {
            placementArray.append(placement.toJson());
        }
        root.insert(QStringLiteral("placements"), placementArray);
        return root;
    }

    static ToolWorkspaceSnapshot fromJson(const QJsonObject &root)
    {
        ToolWorkspaceSnapshot snapshot;
        snapshot.schemaVersion = root.value(QStringLiteral("schemaVersion")).toInt(snapshot.schemaVersion);
        snapshot.workspaceId = root.value(QStringLiteral("workspaceId")).toString();
        snapshot.name = root.value(QStringLiteral("name")).toString();
        snapshot.description = root.value(QStringLiteral("description")).toString();
        snapshot.rootNodeId = root.value(QStringLiteral("rootNodeId")).toString(snapshot.rootNodeId);

        const QDateTime updated = QDateTime::fromString(root.value(QStringLiteral("updatedAt")).toString(),
                                                        Qt::ISODateWithMs);
        if (updated.isValid()) {
            snapshot.updatedAt = updated;
        }

        const QJsonArray nodesArray = root.value(QStringLiteral("nodes")).toArray();
        for (const QJsonValue &value : nodesArray) {
            if (value.isObject()) {
                snapshot.nodes.append(ToolCategoryNode::fromJson(value.toObject()));
            }
        }

        const QJsonArray placementsArray = root.value(QStringLiteral("placements")).toArray();
        for (const QJsonValue &value : placementsArray) {
            if (value.isObject()) {
                snapshot.placements.append(ToolPlacement::fromJson(value.toObject()));
            }
        }

        return snapshot;
    }
};

struct ToolWorkspaceIndexEntry
{
    QString workspaceId;
    QString name;
    QString fileName;
    QDateTime updatedAt = QDateTime::currentDateTimeUtc();

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("workspaceId"), workspaceId);
        root.insert(QStringLiteral("name"), name);
        root.insert(QStringLiteral("fileName"), fileName);
        root.insert(QStringLiteral("updatedAt"), updatedAt.toString(Qt::ISODateWithMs));
        return root;
    }

    static ToolWorkspaceIndexEntry fromJson(const QJsonObject &root)
    {
        ToolWorkspaceIndexEntry entry;
        entry.workspaceId = root.value(QStringLiteral("workspaceId")).toString();
        entry.name = root.value(QStringLiteral("name")).toString();
        entry.fileName = root.value(QStringLiteral("fileName")).toString();
        const QDateTime updated = QDateTime::fromString(root.value(QStringLiteral("updatedAt")).toString(),
                                                        Qt::ISODateWithMs);
        if (updated.isValid()) {
            entry.updatedAt = updated;
        }
        return entry;
    }
};

struct ToolWorkspaceIndex
{
    int schemaVersion = 1;
    QString lastWorkspaceId;
    QVector<ToolWorkspaceIndexEntry> workspaces;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("schemaVersion"), schemaVersion);
        root.insert(QStringLiteral("lastWorkspaceId"), lastWorkspaceId);

        QJsonArray items;
        for (const ToolWorkspaceIndexEntry &entry : workspaces) {
            items.append(entry.toJson());
        }
        root.insert(QStringLiteral("workspaces"), items);
        return root;
    }

    static ToolWorkspaceIndex fromJson(const QJsonObject &root)
    {
        ToolWorkspaceIndex index;
        index.schemaVersion = root.value(QStringLiteral("schemaVersion")).toInt(index.schemaVersion);
        index.lastWorkspaceId = root.value(QStringLiteral("lastWorkspaceId")).toString();

        const QJsonArray items = root.value(QStringLiteral("workspaces")).toArray();
        for (const QJsonValue &value : items) {
            if (value.isObject()) {
                index.workspaces.append(ToolWorkspaceIndexEntry::fromJson(value.toObject()));
            }
        }
        return index;
    }
};

struct ToolImportPackage
{
    int schemaVersion = 1;
    QString packageId;
    QString name;
    QString description;
    QDateTime exportedAt = QDateTime::currentDateTimeUtc();
    QJsonObject exportedBy;
    QJsonObject manifest;
    ToolWorkspaceSnapshot workspace;
    QStringList toolRefs;
    QVector<qtllm::tools::LlmToolDefinition> toolSnapshots;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("schemaVersion"), schemaVersion);
        root.insert(QStringLiteral("packageId"), packageId);
        root.insert(QStringLiteral("name"), name);
        root.insert(QStringLiteral("description"), description);
        root.insert(QStringLiteral("exportedAt"), exportedAt.toString(Qt::ISODateWithMs));
        root.insert(QStringLiteral("exportedBy"), exportedBy);
        root.insert(QStringLiteral("manifest"), manifest);
        root.insert(QStringLiteral("workspace"), workspace.toJson());
        root.insert(QStringLiteral("toolRefs"), detail::toJsonArray(toolRefs));

        QJsonArray toolsArray;
        for (const qtllm::tools::LlmToolDefinition &tool : toolSnapshots) {
            toolsArray.append(tool.toJson());
        }
        root.insert(QStringLiteral("toolSnapshots"), toolsArray);
        return root;
    }

    static ToolImportPackage fromJson(const QJsonObject &root)
    {
        ToolImportPackage pkg;
        pkg.schemaVersion = root.value(QStringLiteral("schemaVersion")).toInt(pkg.schemaVersion);
        pkg.packageId = root.value(QStringLiteral("packageId")).toString();
        pkg.name = root.value(QStringLiteral("name")).toString();
        pkg.description = root.value(QStringLiteral("description")).toString();
        const QDateTime exportedAt = QDateTime::fromString(root.value(QStringLiteral("exportedAt")).toString(),
                                                           Qt::ISODateWithMs);
        if (exportedAt.isValid()) {
            pkg.exportedAt = exportedAt;
        }
        pkg.exportedBy = root.value(QStringLiteral("exportedBy")).toObject();
        pkg.manifest = root.value(QStringLiteral("manifest")).toObject();
        pkg.workspace = ToolWorkspaceSnapshot::fromJson(root.value(QStringLiteral("workspace")).toObject());
        pkg.toolRefs = detail::fromJsonArray(root.value(QStringLiteral("toolRefs")));

        const QJsonArray toolsArray = root.value(QStringLiteral("toolSnapshots")).toArray();
        for (const QJsonValue &value : toolsArray) {
            if (value.isObject()) {
                pkg.toolSnapshots.append(qtllm::tools::LlmToolDefinition::fromJson(value.toObject()));
            }
        }
        return pkg;
    }
};

struct ToolMergeIssue
{
    QString issueType;
    QString severity;
    QString toolId;
    QString message;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("issueType"), issueType);
        root.insert(QStringLiteral("severity"), severity);
        root.insert(QStringLiteral("toolId"), toolId);
        root.insert(QStringLiteral("message"), message);
        return root;
    }
};

struct ToolMergeDecision
{
    QString toolId;
    QString action;
    QString reason;

    QJsonObject toJson() const
    {
        QJsonObject root;
        root.insert(QStringLiteral("toolId"), toolId);
        root.insert(QStringLiteral("action"), action);
        root.insert(QStringLiteral("reason"), reason);
        return root;
    }
};

struct ToolMergePreview
{
    QVector<ToolMergeDecision> decisions;
    QVector<ToolMergeIssue> issues;
    QVector<qtllm::tools::LlmToolDefinition> toolsToInsert;
    ToolWorkspaceSnapshot mergedWorkspace;
    QString summary;

    QJsonObject toJson() const
    {
        QJsonObject root;
        QJsonArray decisionsArray;
        for (const ToolMergeDecision &decision : decisions) {
            decisionsArray.append(decision.toJson());
        }
        root.insert(QStringLiteral("decisions"), decisionsArray);

        QJsonArray issuesArray;
        for (const ToolMergeIssue &issue : issues) {
            issuesArray.append(issue.toJson());
        }
        root.insert(QStringLiteral("issues"), issuesArray);

        QJsonArray insertArray;
        for (const qtllm::tools::LlmToolDefinition &tool : toolsToInsert) {
            insertArray.append(tool.toJson());
        }
        root.insert(QStringLiteral("toolsToInsert"), insertArray);
        root.insert(QStringLiteral("mergedWorkspace"), mergedWorkspace.toJson());
        root.insert(QStringLiteral("summary"), summary);
        return root;
    }
};

} // namespace qtllm::toolsstudio
