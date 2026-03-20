#include "toolstudiosettings.h"

namespace qtllm::toolsstudio {

ToolStudioSettings::ToolStudioSettings()
    : m_settings(QStringLiteral("qtllm"), QStringLiteral("toolstudio"))
{
}

QString ToolStudioSettings::theme() const
{
    return m_settings.value(QStringLiteral("appearance/theme"), QStringLiteral("system")).toString();
}

void ToolStudioSettings::setTheme(const QString &theme)
{
    m_settings.setValue(QStringLiteral("appearance/theme"), theme);
}

QString ToolStudioSettings::uiFontFamily() const
{
    return m_settings.value(QStringLiteral("fonts/uiFamily")).toString();
}

void ToolStudioSettings::setUiFontFamily(const QString &family)
{
    m_settings.setValue(QStringLiteral("fonts/uiFamily"), family);
}

int ToolStudioSettings::uiFontPointSize() const
{
    return m_settings.value(QStringLiteral("fonts/uiPointSize"), 10).toInt();
}

void ToolStudioSettings::setUiFontPointSize(int size)
{
    m_settings.setValue(QStringLiteral("fonts/uiPointSize"), size);
}

QString ToolStudioSettings::editorFontFamily() const
{
    return m_settings.value(QStringLiteral("fonts/editorFamily"), QStringLiteral("Consolas")).toString();
}

void ToolStudioSettings::setEditorFontFamily(const QString &family)
{
    m_settings.setValue(QStringLiteral("fonts/editorFamily"), family);
}

int ToolStudioSettings::editorFontPointSize() const
{
    return m_settings.value(QStringLiteral("fonts/editorPointSize"), 10).toInt();
}

void ToolStudioSettings::setEditorFontPointSize(int size)
{
    m_settings.setValue(QStringLiteral("fonts/editorPointSize"), size);
}

QString ToolStudioSettings::lastWorkspaceId() const
{
    return m_settings.value(QStringLiteral("workspace/lastWorkspaceId"), QStringLiteral("default")).toString();
}

void ToolStudioSettings::setLastWorkspaceId(const QString &workspaceId)
{
    m_settings.setValue(QStringLiteral("workspace/lastWorkspaceId"), workspaceId);
}

QByteArray ToolStudioSettings::mainWindowGeometry() const
{
    return m_settings.value(QStringLiteral("window/geometry")).toByteArray();
}

void ToolStudioSettings::setMainWindowGeometry(const QByteArray &data)
{
    m_settings.setValue(QStringLiteral("window/geometry"), data);
}

QByteArray ToolStudioSettings::mainWindowState() const
{
    return m_settings.value(QStringLiteral("window/state")).toByteArray();
}

void ToolStudioSettings::setMainWindowState(const QByteArray &data)
{
    m_settings.setValue(QStringLiteral("window/state"), data);
}

bool ToolStudioSettings::showBuiltinTools() const
{
    return m_settings.value(QStringLiteral("filters/showBuiltinTools"), true).toBool();
}

void ToolStudioSettings::setShowBuiltinTools(bool enabled)
{
    m_settings.setValue(QStringLiteral("filters/showBuiltinTools"), enabled);
}

bool ToolStudioSettings::showDisabledTools() const
{
    return m_settings.value(QStringLiteral("filters/showDisabledTools"), true).toBool();
}

void ToolStudioSettings::setShowDisabledTools(bool enabled)
{
    m_settings.setValue(QStringLiteral("filters/showDisabledTools"), enabled);
}

bool ToolStudioSettings::includeDescendantsByDefault() const
{
    return m_settings.value(QStringLiteral("filters/includeDescendants"), true).toBool();
}

void ToolStudioSettings::setIncludeDescendantsByDefault(bool enabled)
{
    m_settings.setValue(QStringLiteral("filters/includeDescendants"), enabled);
}

} // namespace qtllm::toolsstudio
