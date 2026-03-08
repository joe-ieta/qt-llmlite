#pragma once

#include "llmtooldefinition.h"

#include "../core/llmtypes.h"
#include "../profile/clientprofile.h"

#include <QList>

namespace qtllm::tools {

class ToolSelectionLayer
{
public:
    QList<LlmToolDefinition> selectTools(const profile::ClientProfile &profile,
                                         const QVector<LlmMessage> &history,
                                         const QList<LlmToolDefinition> &availableTools,
                                         int maxTools = 8) const;
};

} // namespace qtllm::tools
