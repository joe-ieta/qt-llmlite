#include "toolcatalogservice.h"

#include "../tools/builtintools.h"

namespace qtllm::toolsstudio {

ToolCatalogService::ToolCatalogService(std::shared_ptr<qtllm::tools::LlmToolRegistry> registry,
                                       std::shared_ptr<qtllm::tools::runtime::ToolCatalogRepository> repository,
                                       std::shared_ptr<ToolMetadataOverrideRepository> overrideRepository)
    : m_registry(std::move(registry))
    , m_repository(std::move(repository))
    , m_overrideRepository(std::move(overrideRepository))
{
}

bool ToolCatalogService::load(QString *errorMessage)
{
    if (!m_registry || !m_repository) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Tool catalog service is not fully configured");
        }
        return false;
    }

    QString catalogLoadError;
    const std::optional<qtllm::tools::runtime::ToolCatalogSnapshot> snapshot =
        m_repository->loadCatalog(&catalogLoadError);
    if (snapshot.has_value()) {
        QList<qtllm::tools::LlmToolDefinition> tools;
        for (const qtllm::tools::LlmToolDefinition &tool : snapshot->tools) {
            tools.append(tool);
        }
        m_registry->replaceAll(tools);
    } else if (catalogLoadError.isEmpty()) {
        ensureBuiltInsPresent();
    } else {
        if (errorMessage) {
            *errorMessage = catalogLoadError;
        }
        return false;
    }

    return loadOverrides(errorMessage);
}

bool ToolCatalogService::save(QString *errorMessage) const
{
    if (!m_registry || !m_repository) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Tool catalog service is not fully configured");
        }
        return false;
    }

    qtllm::tools::runtime::ToolCatalogSnapshot snapshot;
    const QList<qtllm::tools::LlmToolDefinition> tools = m_registry->allTools();
    for (const qtllm::tools::LlmToolDefinition &tool : tools) {
        snapshot.tools.append(tool);
    }
    if (!m_repository->saveCatalog(snapshot, errorMessage)) {
        return false;
    }

    return saveOverrides(errorMessage);
}

bool ToolCatalogService::reloadFromDisk(QString *errorMessage)
{
    return load(errorMessage);
}

QList<qtllm::tools::LlmToolDefinition> ToolCatalogService::allTools() const
{
    if (!m_registry) {
        return QList<qtllm::tools::LlmToolDefinition>();
    }

    QList<qtllm::tools::LlmToolDefinition> tools = m_registry->allTools();
    for (qtllm::tools::LlmToolDefinition &tool : tools) {
        tool = applyMetadataOverride(tool);
    }
    return tools;
}

std::optional<qtllm::tools::LlmToolDefinition> ToolCatalogService::findTool(const QString &toolId) const
{
    if (!m_registry) {
        return std::nullopt;
    }

    const QString resolved = m_registry->resolveToolId(toolId);
    const QList<qtllm::tools::LlmToolDefinition> tools = allTools();
    for (const qtllm::tools::LlmToolDefinition &tool : tools) {
        if (tool.toolId == resolved) {
            return tool;
        }
    }

    return std::nullopt;
}

bool ToolCatalogService::containsTool(const QString &toolId) const
{
    return m_registry && m_registry->contains(toolId);
}

bool ToolCatalogService::canEditTool(const QString &toolId) const
{
    Q_UNUSED(toolId)
    return true;
}

bool ToolCatalogService::isInvocationNameEditable(const QString &toolId) const
{
    Q_UNUSED(toolId)
    return false;
}

bool ToolCatalogService::updateEditableFields(const QString &toolId,
                                              const ToolStudioEditableFields &fields,
                                              QString *errorMessage)
{
    if (!m_registry) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Tool registry is not configured");
        }
        return false;
    }

    const QString resolved = m_registry->resolveToolId(toolId);
    const ToolStudioEditableFields sanitizedFields = sanitizeEditableFields(fields);

    if (isMcpToolId(resolved)) {
        if (!m_registry->contains(resolved)) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Tool not found: ") + resolved;
            }
            return false;
        }

        m_metadataOverrides.insert(resolved, sanitizedFields);
        return save(errorMessage);
    }

    QList<qtllm::tools::LlmToolDefinition> tools = m_registry->allTools();
    bool found = false;
    for (qtllm::tools::LlmToolDefinition &tool : tools) {
        if (tool.toolId != resolved) {
            continue;
        }

        tool.name = sanitizedFields.name;
        tool.description = sanitizedFields.description;
        tool.inputSchema = sanitizedFields.inputSchema;
        tool.capabilityTags = sanitizedFields.capabilityTags;
        tool.enabled = sanitizedFields.enabled;
        found = true;
        break;
    }

    if (!found) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Tool not found: ") + resolved;
        }
        return false;
    }

    m_registry->replaceAll(tools);
    return save(errorMessage);
}

std::shared_ptr<qtllm::tools::LlmToolRegistry> ToolCatalogService::registry() const
{
    return m_registry;
}

void ToolCatalogService::ensureBuiltInsPresent()
{
    if (!m_registry) {
        return;
    }

    bool changed = false;
    const QList<qtllm::tools::LlmToolDefinition> merged =
        qtllm::tools::mergeWithBuiltInTools(m_registry->allTools(), &changed);
    if (changed) {
        m_registry->replaceAll(merged);
    }
}

qtllm::tools::LlmToolDefinition ToolCatalogService::applyMetadataOverride(const qtllm::tools::LlmToolDefinition &tool) const
{
    const auto it = m_metadataOverrides.constFind(tool.toolId);
    if (it == m_metadataOverrides.constEnd()) {
        return tool;
    }

    qtllm::tools::LlmToolDefinition merged = tool;
    const ToolStudioEditableFields &fields = it.value();
    merged.name = fields.name;
    merged.description = fields.description;
    merged.inputSchema = fields.inputSchema;
    merged.capabilityTags = fields.capabilityTags;
    merged.enabled = fields.enabled;
    return merged;
}

ToolStudioEditableFields ToolCatalogService::sanitizeEditableFields(const ToolStudioEditableFields &fields) const
{
    ToolStudioEditableFields sanitized = fields;
    sanitized.name = sanitized.name.trimmed();
    sanitized.description = sanitized.description.trimmed();

    QStringList cleanedTags;
    for (const QString &tag : sanitized.capabilityTags) {
        const QString trimmed = tag.trimmed();
        if (!trimmed.isEmpty()) {
            cleanedTags.append(trimmed);
        }
    }
    cleanedTags.removeDuplicates();
    sanitized.capabilityTags = cleanedTags;
    return sanitized;
}

bool ToolCatalogService::isMcpToolId(const QString &toolId) const
{
    return toolId.startsWith(QStringLiteral("mcp::"));
}

bool ToolCatalogService::loadOverrides(QString *errorMessage)
{
    m_metadataOverrides.clear();
    if (!m_overrideRepository) {
        return true;
    }

    QString overrideLoadError;
    const std::optional<ToolMetadataOverridesSnapshot> snapshot =
        m_overrideRepository->loadOverrides(&overrideLoadError);
    if (!snapshot.has_value()) {
        if (!overrideLoadError.isEmpty()) {
            if (errorMessage) {
                *errorMessage = overrideLoadError;
            }
            return false;
        }
        return true;
    }

    for (const ToolMetadataOverride &entry : snapshot->overrides) {
        const QString toolId = entry.toolId.trimmed();
        if (!toolId.isEmpty()) {
            m_metadataOverrides.insert(toolId, sanitizeEditableFields(entry.fields));
        }
    }

    return true;
}

bool ToolCatalogService::saveOverrides(QString *errorMessage) const
{
    if (!m_overrideRepository) {
        return true;
    }

    ToolMetadataOverridesSnapshot snapshot;
    for (auto it = m_metadataOverrides.constBegin(); it != m_metadataOverrides.constEnd(); ++it) {
        ToolMetadataOverride entry;
        entry.toolId = it.key();
        entry.fields = it.value();
        snapshot.overrides.append(entry);
    }

    return m_overrideRepository->saveOverrides(snapshot, errorMessage);
}

} // namespace qtllm::toolsstudio
