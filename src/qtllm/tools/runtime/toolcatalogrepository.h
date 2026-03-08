#pragma once

#include "toolruntime_types.h"

#include <optional>

namespace qtllm::tools::runtime {

class ToolCatalogRepository
{
public:
    explicit ToolCatalogRepository(QString rootDirectory = QStringLiteral(".qtllm/tools"));

    bool saveCatalog(const ToolCatalogSnapshot &snapshot, QString *errorMessage = nullptr) const;
    std::optional<ToolCatalogSnapshot> loadCatalog(QString *errorMessage = nullptr) const;

private:
    QString catalogPath() const;
    bool ensureRootDirectory(QString *errorMessage = nullptr) const;

private:
    QString m_rootDirectory;
};

} // namespace qtllm::tools::runtime
