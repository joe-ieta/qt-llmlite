#pragma once

#include "toolsinsideartifactstore.h"
#include "toolsinsiderepository.h"

#include <QString>

#include <memory>

namespace qtllm::toolsinside {

class ToolsInsideAdminService
{
public:
    explicit ToolsInsideAdminService(QString rootDirectory = QStringLiteral(".qtllm/tools_inside"),
                                     std::shared_ptr<ToolsInsideRepository> repository = nullptr,
                                     std::shared_ptr<ToolsInsideArtifactStore> artifactStore = nullptr);

    void setRootDirectory(const QString &rootDirectory);
    QString rootDirectory() const;

    void setRepository(const std::shared_ptr<ToolsInsideRepository> &repository);
    void setArtifactStore(const std::shared_ptr<ToolsInsideArtifactStore> &artifactStore);

    bool archiveTrace(const QString &traceId, QString *errorMessage = nullptr) const;
    bool purgeTrace(const QString &traceId, QString *errorMessage = nullptr) const;

private:
    QString m_rootDirectory;
    std::shared_ptr<ToolsInsideRepository> m_repository;
    std::shared_ptr<ToolsInsideArtifactStore> m_artifactStore;
};

} // namespace qtllm::toolsinside
