#include "llmtoolregistry.h"

#include "builtintools.h"

namespace qtllm::tools {

LlmToolRegistry::LlmToolRegistry()
{
    const QList<LlmToolDefinition> defaults = builtInTools();
    for (const LlmToolDefinition &tool : defaults) {
        registerTool(tool);
    }
}

bool LlmToolRegistry::registerTool(const LlmToolDefinition &tool)
{
    if (tool.toolId.trimmed().isEmpty()) {
        return false;
    }

    LlmToolDefinition normalized = tool;
    if (normalized.systemBuiltIn) {
        normalized.category = QStringLiteral("builtin");
        normalized.removable = false;
        normalized.enabled = true;
    }

    m_tools.insert(normalized.toolId, normalized);
    return true;
}

bool LlmToolRegistry::unregisterTool(const QString &toolId)
{
    const QString id = toolId.trimmed();
    if (id.isEmpty()) {
        return false;
    }

    const auto it = m_tools.constFind(id);
    if (it != m_tools.constEnd() && !it->removable) {
        return false;
    }

    return m_tools.remove(id) > 0;
}

void LlmToolRegistry::clear()
{
    m_tools.clear();
    const QList<LlmToolDefinition> defaults = builtInTools();
    for (const LlmToolDefinition &tool : defaults) {
        registerTool(tool);
    }
}

void LlmToolRegistry::replaceAll(const QList<LlmToolDefinition> &tools)
{
    m_tools.clear();
    const QList<LlmToolDefinition> merged = mergeWithBuiltInTools(tools);
    for (const LlmToolDefinition &tool : merged) {
        if (!tool.toolId.trimmed().isEmpty()) {
            registerTool(tool);
        }
    }
}

QList<LlmToolDefinition> LlmToolRegistry::allTools() const
{
    return m_tools.values();
}

QList<LlmToolDefinition> LlmToolRegistry::enabledTools() const
{
    QList<LlmToolDefinition> result;
    for (const LlmToolDefinition &tool : m_tools) {
        if (tool.enabled) {
            result.append(tool);
        }
    }
    return result;
}

bool LlmToolRegistry::contains(const QString &toolId) const
{
    return m_tools.contains(toolId.trimmed());
}

bool LlmToolRegistry::isRemovable(const QString &toolId) const
{
    const auto it = m_tools.constFind(toolId.trimmed());
    if (it == m_tools.constEnd()) {
        return false;
    }
    return it->removable;
}

} // namespace qtllm::tools
