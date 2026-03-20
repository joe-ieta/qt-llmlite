#pragma once

#include "toolsstudio_types.h"
#include "toolmetadataoverriderepository.h"

#include "../tools/llmtoolregistry.h"
#include "../tools/runtime/toolcatalogrepository.h"

#include <QHash>
#include <memory>
#include <optional>

namespace qtllm::toolsstudio {

class ToolCatalogService
{
public:
    ToolCatalogService(std::shared_ptr<qtllm::tools::LlmToolRegistry> registry = std::make_shared<qtllm::tools::LlmToolRegistry>(),
                       std::shared_ptr<qtllm::tools::runtime::ToolCatalogRepository> repository = std::make_shared<qtllm::tools::runtime::ToolCatalogRepository>(),
                       std::shared_ptr<ToolMetadataOverrideRepository> overrideRepository = std::make_shared<ToolMetadataOverrideRepository>());

    bool load(QString *errorMessage = nullptr);
    bool save(QString *errorMessage = nullptr) const;
    bool reloadFromDisk(QString *errorMessage = nullptr);

    QList<qtllm::tools::LlmToolDefinition> allTools() const;
    std::optional<qtllm::tools::LlmToolDefinition> findTool(const QString &toolId) const;
    bool containsTool(const QString &toolId) const;

    bool canEditTool(const QString &toolId) const;
    bool isInvocationNameEditable(const QString &toolId) const;

    bool updateEditableFields(const QString &toolId,
                              const ToolStudioEditableFields &fields,
                              QString *errorMessage = nullptr);

    std::shared_ptr<qtllm::tools::LlmToolRegistry> registry() const;

private:
    void ensureBuiltInsPresent();
    qtllm::tools::LlmToolDefinition applyMetadataOverride(const qtllm::tools::LlmToolDefinition &tool) const;
    ToolStudioEditableFields sanitizeEditableFields(const ToolStudioEditableFields &fields) const;
    bool isMcpToolId(const QString &toolId) const;
    bool loadOverrides(QString *errorMessage = nullptr);
    bool saveOverrides(QString *errorMessage = nullptr) const;

private:
    std::shared_ptr<qtllm::tools::LlmToolRegistry> m_registry;
    std::shared_ptr<qtllm::tools::runtime::ToolCatalogRepository> m_repository;
    std::shared_ptr<ToolMetadataOverrideRepository> m_overrideRepository;
    QHash<QString, ToolStudioEditableFields> m_metadataOverrides;
};

} // namespace qtllm::toolsstudio
