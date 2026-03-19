#pragma once

#include "toolsstudio_types.h"

#include <optional>

namespace qtllm::toolsstudio {

class ToolMetadataOverrideRepository
{
public:
    explicit ToolMetadataOverrideRepository(QString rootDirectory = QStringLiteral(".qtllm/tools/studio"));

    bool saveOverrides(const ToolMetadataOverridesSnapshot &snapshot, QString *errorMessage = nullptr) const;
    std::optional<ToolMetadataOverridesSnapshot> loadOverrides(QString *errorMessage = nullptr) const;

private:
    QString overridesPath() const;
    bool ensureRootDirectory(QString *errorMessage) const;

private:
    QString m_rootDirectory;
};

} // namespace qtllm::toolsstudio
