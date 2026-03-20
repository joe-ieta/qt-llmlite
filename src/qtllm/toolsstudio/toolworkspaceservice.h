#pragma once

#include "toolsstudio_types.h"
#include "toolworkspacerepository.h"

#include <QSet>
#include <memory>
#include <optional>

namespace qtllm::toolsstudio {

class ToolWorkspaceService
{
public:
    explicit ToolWorkspaceService(std::shared_ptr<ToolWorkspaceRepository> repository = std::make_shared<ToolWorkspaceRepository>());

    bool initialize(QString preferredWorkspaceId = QStringLiteral("default"), QString *errorMessage = nullptr);
    bool loadWorkspace(const QString &workspaceId, QString *errorMessage = nullptr);
    bool saveWorkspace(QString *errorMessage = nullptr);
    bool replaceWorkspace(const ToolWorkspaceSnapshot &workspace, QString *errorMessage = nullptr);

    ToolWorkspaceSnapshot currentWorkspace() const;
    QString currentWorkspaceId() const;
    QVector<ToolWorkspaceIndexEntry> workspaceEntries(QString *errorMessage = nullptr) const;

    QVector<ToolCategoryNode> allNodes() const;
    QVector<ToolPlacement> allPlacements() const;
    QVector<ToolCategoryNode> childNodes(const QString &parentNodeId) const;
    QVector<ToolPlacement> placementsInNode(const QString &nodeId, bool includeDescendants = false) const;
    std::optional<ToolCategoryNode> findNode(const QString &nodeId) const;
    std::optional<ToolPlacement> findPlacement(const QString &placementId) const;

    bool createWorkspace(const QString &name, QString *outWorkspaceId = nullptr, QString *errorMessage = nullptr);
    bool removeWorkspace(const QString &workspaceId, QString *errorMessage = nullptr);

    bool createNode(const QString &parentNodeId, const QString &name, QString *outNodeId = nullptr, QString *errorMessage = nullptr);
    bool updateNode(const ToolCategoryNode &node, QString *errorMessage = nullptr);
    bool moveNode(const QString &nodeId, const QString &newParentNodeId, int newOrder, QString *errorMessage = nullptr);
    bool reorderNodeChildren(const QString &parentNodeId, const QStringList &orderedNodeIds, QString *errorMessage = nullptr);
    bool removeNode(const QString &nodeId, bool recursive, QString *errorMessage = nullptr);

    bool addToolToNode(const QString &nodeId, const QString &toolId, QString *outPlacementId = nullptr, QString *errorMessage = nullptr);
    bool updatePlacement(const ToolPlacement &placement, QString *errorMessage = nullptr);
    bool movePlacement(const QString &placementId, const QString &targetNodeId, int newOrder, QString *errorMessage = nullptr);
    bool reorderPlacements(const QString &nodeId, const QStringList &orderedPlacementIds, QString *errorMessage = nullptr);
    bool removePlacement(const QString &placementId, QString *errorMessage = nullptr);

private:
    ToolWorkspaceSnapshot defaultWorkspace(const QString &workspaceId, const QString &name) const;
    bool saveIndex(const QString &lastWorkspaceId, QString *errorMessage = nullptr) const;
    ToolWorkspaceIndex loadOrCreateIndex(QString *errorMessage = nullptr) const;
    bool nodeExists(const QString &nodeId) const;
    bool nodeContainsTool(const QString &nodeId, const QString &toolId) const;
    QSet<QString> descendantNodeIds(const QString &nodeId) const;
    QString generateWorkspaceId(const QString &name) const;
    QString generateNodeId() const;
    QString generatePlacementId() const;

private:
    std::shared_ptr<ToolWorkspaceRepository> m_repository;
    ToolWorkspaceSnapshot m_currentWorkspace;
};

} // namespace qtllm::toolsstudio
