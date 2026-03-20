#pragma once

#include "toolstudiocontroller.h"

#include <QMainWindow>

class QAction;
class QCheckBox;
class QLineEdit;
class QPlainTextEdit;
class QTableWidget;
class QTextEdit;
class QTreeWidget;

class ToolStudioWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ToolStudioWindow(QWidget *parent = nullptr);
    ~ToolStudioWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void refreshUi();
    void onTreeSelectionChanged();
    void onTableSelectionChanged();
    void importPackage();
    void exportWorkspace();
    void exportSelectedSubtree();
    void createWorkspace();
    void openWorkspace();
    void removeWorkspace();
    void createCategory();
    void renameCategory();
    void moveCategoryTo();
    void moveCategoryUp();
    void moveCategoryDown();
    void removeCategory();
    void addToolToCategory();
    void moveSelectedPlacementToCategory();
    void moveSelectedPlacementUp();
    void moveSelectedPlacementDown();
    void removeSelectedPlacement();
    void saveEdits();
    void saveAll();
    void openSettings();

private:
    void buildUi();
    void buildMenus();
    void configureDragDrop();
    void applySettings();
    void populateTree();
    void populateTable();
    void populateEditors();
    void handleCategoryDrop(const QString &nodeId, const QString &targetParentId, int insertIndex);
    void handleToolDropToCategory(const QString &toolId, const QString &placementId, const QString &targetNodeId);
    void handlePlacementReorder(const QString &placementId, int insertIndex);
    bool isCatalogSelection() const;
    QString chooseCategory(const QString &title, const QString &label, const QString &excludeNodeId = QString()) const;
    QString currentNodeId() const;
    QString selectedToolId() const;
    QString selectedPlacementId() const;
    bool currentToolFieldsFromUi(qtllm::toolsstudio::ToolStudioEditableFields *fields,
                                 QString *errorMessage = nullptr) const;
    qtllm::toolsstudio::ToolPlacement currentPlacementFromUi() const;
    QString sourceLabelForTool(const qtllm::tools::LlmToolDefinition &tool) const;
    void showError(const QString &message);
    void showStatus(const QString &message);

private:
    ToolStudioController *m_controller;
    QTreeWidget *m_tree;
    QTableWidget *m_table;
    QCheckBox *m_includeDescendantsCheck;
    QLineEdit *m_nameEdit;
    QLineEdit *m_toolIdEdit;
    QLineEdit *m_invocationEdit;
    QLineEdit *m_globalTagsEdit;
    QCheckBox *m_enabledCheck;
    QTextEdit *m_descriptionEdit;
    QPlainTextEdit *m_schemaEdit;
    QLineEdit *m_localTagsEdit;
    QTextEdit *m_noteEdit;
    QCheckBox *m_pinnedCheck;
    QAction *m_saveAction;
    QAction *m_importAction;
    QAction *m_exportWorkspaceAction;
    QAction *m_exportSubtreeAction;
    QAction *m_createWorkspaceAction;
    QAction *m_openWorkspaceAction;
    QAction *m_removeWorkspaceAction;
    QAction *m_createCategoryAction;
    QAction *m_renameCategoryAction;
    QAction *m_moveCategoryAction;
    QAction *m_moveCategoryUpAction;
    QAction *m_moveCategoryDownAction;
    QAction *m_removeCategoryAction;
    QAction *m_addToolAction;
    QAction *m_movePlacementAction;
    QAction *m_movePlacementUpAction;
    QAction *m_movePlacementDownAction;
    QAction *m_removePlacementAction;
    QAction *m_settingsAction;
};
