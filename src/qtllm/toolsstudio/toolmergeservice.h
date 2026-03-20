#pragma once

#include "toolsstudio_types.h"

namespace qtllm::toolsstudio {

class ToolCatalogService;
class ToolWorkspaceService;

class ToolMergeService
{
public:
    ToolMergePreview buildPreview(const ToolImportPackage &package,
                                  const QList<qtllm::tools::LlmToolDefinition> &localTools,
                                  const ToolWorkspaceSnapshot &localWorkspace) const;

    bool applyPreview(const ToolMergePreview &preview,
                      ToolCatalogService *catalogService,
                      ToolWorkspaceService *workspaceService,
                      QString *errorMessage = nullptr) const;
};

} // namespace qtllm::toolsstudio
