#pragma once

#include <QByteArray>
#include <QSettings>
#include <QString>

namespace qtllm::toolsstudio {

class ToolStudioSettings
{
public:
    ToolStudioSettings();

    QString theme() const;
    void setTheme(const QString &theme);

    QString uiFontFamily() const;
    void setUiFontFamily(const QString &family);

    int uiFontPointSize() const;
    void setUiFontPointSize(int size);

    QString editorFontFamily() const;
    void setEditorFontFamily(const QString &family);

    int editorFontPointSize() const;
    void setEditorFontPointSize(int size);

    QString lastWorkspaceId() const;
    void setLastWorkspaceId(const QString &workspaceId);

    QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray &data);

    QByteArray mainWindowState() const;
    void setMainWindowState(const QByteArray &data);

    bool showBuiltinTools() const;
    void setShowBuiltinTools(bool enabled);

    bool showDisabledTools() const;
    void setShowDisabledTools(bool enabled);

    bool includeDescendantsByDefault() const;
    void setIncludeDescendantsByDefault(bool enabled);

private:
    QSettings m_settings;
};

} // namespace qtllm::toolsstudio
