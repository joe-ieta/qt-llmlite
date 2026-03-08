#pragma once

#include "llmtooldefinition.h"

#include <QList>
#include <QString>

namespace qtllm::tools {

QList<LlmToolDefinition> builtInTools();
bool isBuiltInToolId(const QString &toolId);
QList<LlmToolDefinition> mergeWithBuiltInTools(const QList<LlmToolDefinition> &tools, bool *changed = nullptr);

} // namespace qtllm::tools
