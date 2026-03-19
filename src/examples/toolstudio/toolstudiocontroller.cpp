#include "toolstudiocontroller.h"

ToolStudioController::ToolStudioController(QObject *parent)
    : QObject(parent)
    , m_catalogService(std::make_unique<qtllm::toolsstudio::ToolCatalogService>())
    , m_importExportService(std::make_unique<qtllm::toolsstudio::ToolImportExportService>())
    , m_mergeService(std::make_unique<qtllm::toolsstudio::ToolMergeService>())
    , m_workspaceService(std::make_unique<qtllm::toolsstudio::ToolWorkspaceService>())
    , m_settings(std::make_unique<qtllm::toolsstudio::ToolStudioSettings>())
{
}

bool ToolStudioController::initialize(QString *errorMessage)
{
    if (!m_catalogService->load(errorMessage)) {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to load tool catalog"));
        return false;
    }

    if (!m_workspaceService->initialize(m_settings->lastWorkspaceId(), errorMessage)) {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to initialize workspace"));
        return false;
    }

    m_settings->setLastWorkspaceId(m_workspaceService->currentWorkspaceId());
    emit stateChanged();
    return true;
}

qtllm::toolsstudio::ToolCatalogService *ToolStudioController::catalogService() const
{
    return m_catalogService.get();
}

qtllm::toolsstudio::ToolWorkspaceService *ToolStudioController::workspaceService() const
{
    return m_workspaceService.get();
}

qtllm::toolsstudio::ToolStudioSettings *ToolStudioController::settings() const
{
    return m_settings.get();
}

bool ToolStudioController::saveAll(QString *errorMessage)
{
    const bool okCatalog = m_catalogService->save(errorMessage);
    const bool okWorkspace = m_workspaceService->saveWorkspace(errorMessage);
    if (okCatalog && okWorkspace) {
        emit statusMessage(QStringLiteral("Saved catalog and workspace."));
        emit stateChanged();
        return true;
    }

    emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to save state"));
    return false;
}

bool ToolStudioController::createWorkspace(const QString &name, QString *outWorkspaceId, QString *errorMessage)
{
    const bool ok = m_workspaceService->createWorkspace(name, outWorkspaceId, errorMessage);
    if (ok) {
        m_settings->setLastWorkspaceId(m_workspaceService->currentWorkspaceId());
        emit statusMessage(QStringLiteral("Workspace created."));
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to create workspace"));
    }
    return ok;
}

bool ToolStudioController::openWorkspace(const QString &workspaceId, QString *errorMessage)
{
    const bool ok = m_workspaceService->loadWorkspace(workspaceId, errorMessage);
    if (ok) {
        m_settings->setLastWorkspaceId(workspaceId);
        emit statusMessage(QStringLiteral("Workspace opened."));
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to open workspace"));
    }
    return ok;
}

bool ToolStudioController::removeWorkspace(const QString &workspaceId, QString *errorMessage)
{
    const bool ok = m_workspaceService->removeWorkspace(workspaceId, errorMessage);
    if (ok) {
        emit statusMessage(QStringLiteral("Workspace removed."));
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to remove workspace"));
    }
    return ok;
}

bool ToolStudioController::createNode(const QString &parentNodeId,
                                      const QString &name,
                                      QString *outNodeId,
                                      QString *errorMessage)
{
    const bool ok = m_workspaceService->createNode(parentNodeId, name, outNodeId, errorMessage);
    if (ok) {
        emit statusMessage(QStringLiteral("Category created."));
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to create category"));
    }
    return ok;
}

bool ToolStudioController::updateNode(const qtllm::toolsstudio::ToolCategoryNode &node, QString *errorMessage)
{
    const bool ok = m_workspaceService->updateNode(node, errorMessage);
    if (ok) {
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to update category"));
    }
    return ok;
}

bool ToolStudioController::moveNode(const QString &nodeId,
                                    const QString &newParentNodeId,
                                    int newOrder,
                                    QString *errorMessage)
{
    const bool ok = m_workspaceService->moveNode(nodeId, newParentNodeId, newOrder, errorMessage);
    if (ok) {
        emit statusMessage(QStringLiteral("Category moved."));
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to move category"));
    }
    return ok;
}

bool ToolStudioController::reorderNodeChildren(const QString &parentNodeId,
                                               const QStringList &orderedNodeIds,
                                               QString *errorMessage)
{
    const bool ok = m_workspaceService->reorderNodeChildren(parentNodeId, orderedNodeIds, errorMessage);
    if (ok) {
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to reorder categories"));
    }
    return ok;
}

bool ToolStudioController::removeNode(const QString &nodeId, bool recursive, QString *errorMessage)
{
    const bool ok = m_workspaceService->removeNode(nodeId, recursive, errorMessage);
    if (ok) {
        emit statusMessage(QStringLiteral("Category removed."));
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to remove category"));
    }
    return ok;
}

bool ToolStudioController::addToolToNode(const QString &nodeId,
                                         const QString &toolId,
                                         QString *outPlacementId,
                                         QString *errorMessage)
{
    const bool ok = m_workspaceService->addToolToNode(nodeId, toolId, outPlacementId, errorMessage);
    if (ok) {
        emit statusMessage(QStringLiteral("Tool added to category."));
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to add tool to category"));
    }
    return ok;
}

bool ToolStudioController::updatePlacement(const qtllm::toolsstudio::ToolPlacement &placement, QString *errorMessage)
{
    const bool ok = m_workspaceService->updatePlacement(placement, errorMessage);
    if (ok) {
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to update placement"));
    }
    return ok;
}

bool ToolStudioController::movePlacement(const QString &placementId,
                                         const QString &targetNodeId,
                                         int newOrder,
                                         QString *errorMessage)
{
    const bool ok = m_workspaceService->movePlacement(placementId, targetNodeId, newOrder, errorMessage);
    if (ok) {
        emit statusMessage(QStringLiteral("Tool placement moved."));
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to move tool placement"));
    }
    return ok;
}

bool ToolStudioController::reorderPlacements(const QString &nodeId,
                                             const QStringList &orderedPlacementIds,
                                             QString *errorMessage)
{
    const bool ok = m_workspaceService->reorderPlacements(nodeId, orderedPlacementIds, errorMessage);
    if (ok) {
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to reorder tool placements"));
    }
    return ok;
}

bool ToolStudioController::removePlacement(const QString &placementId, QString *errorMessage)
{
    const bool ok = m_workspaceService->removePlacement(placementId, errorMessage);
    if (ok) {
        emit statusMessage(QStringLiteral("Tool removed from category."));
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to remove tool from category"));
    }
    return ok;
}

bool ToolStudioController::updateToolFields(const QString &toolId,
                                            const qtllm::toolsstudio::ToolStudioEditableFields &fields,
                                            QString *errorMessage)
{
    const bool ok = m_catalogService->updateEditableFields(toolId, fields, errorMessage);
    if (ok) {
        emit statusMessage(QStringLiteral("Tool metadata updated."));
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to update tool"));
    }
    return ok;
}

bool ToolStudioController::exportWorkspace(const QString &filePath, bool includeToolSnapshots, QString *errorMessage)
{
    const bool ok = m_importExportService->exportWorkspace(m_workspaceService->currentWorkspace(),
                                                           m_catalogService->allTools(),
                                                           filePath,
                                                           includeToolSnapshots,
                                                           errorMessage);
    if (ok) {
        emit statusMessage(QStringLiteral("Workspace exported."));
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to export workspace"));
    }
    return ok;
}

bool ToolStudioController::exportSubtree(const QString &nodeId,
                                         const QString &filePath,
                                         bool includeToolSnapshots,
                                         QString *errorMessage)
{
    const bool ok = m_importExportService->exportSubtree(m_workspaceService->currentWorkspace(),
                                                         nodeId,
                                                         m_catalogService->allTools(),
                                                         filePath,
                                                         includeToolSnapshots,
                                                         errorMessage);
    if (ok) {
        emit statusMessage(QStringLiteral("Subtree exported."));
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to export subtree"));
    }
    return ok;
}

std::optional<qtllm::toolsstudio::ToolMergePreview> ToolStudioController::buildImportPreview(const QString &filePath,
                                                                                              QString *errorMessage) const
{
    const std::optional<qtllm::toolsstudio::ToolImportPackage> package =
        m_importExportService->loadPackage(filePath, errorMessage);
    if (!package.has_value()) {
        return std::nullopt;
    }

    return m_mergeService->buildPreview(*package,
                                        m_catalogService->allTools(),
                                        m_workspaceService->currentWorkspace());
}

bool ToolStudioController::applyImportPreview(const qtllm::toolsstudio::ToolMergePreview &preview, QString *errorMessage)
{
    const bool ok = m_mergeService->applyPreview(preview,
                                                 m_catalogService.get(),
                                                 m_workspaceService.get(),
                                                 errorMessage);
    if (ok) {
        emit statusMessage(QStringLiteral("Import merged into current workspace."));
        emit stateChanged();
    } else {
        emit errorOccurred(errorMessage ? *errorMessage : QStringLiteral("Failed to apply import"));
    }
    return ok;
}
