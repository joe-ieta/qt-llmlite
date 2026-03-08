#pragma once

#include "llmtooldefinition.h"

#include <QHash>
#include <QList>
#include <QString>

namespace qtllm::tools {

class LlmToolRegistry
{
public:
    bool registerTool(const LlmToolDefinition &tool);
    bool unregisterTool(const QString &toolId);
    void clear();
    void replaceAll(const QList<LlmToolDefinition> &tools);

    QList<LlmToolDefinition> allTools() const;
    QList<LlmToolDefinition> enabledTools() const;
    bool contains(const QString &toolId) const;
    bool isRemovable(const QString &toolId) const;

private:
    QHash<QString, LlmToolDefinition> m_tools;
};

} // namespace qtllm::tools
