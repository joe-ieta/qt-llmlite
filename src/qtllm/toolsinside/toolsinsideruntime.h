#pragma once

#include "toolsinsideadminservice.h"
#include "toolsinsideartifactstore.h"
#include "toolsinsidequeryservice.h"
#include "toolsinsiderepository.h"
#include "toolsinside_types.h"

#include <memory>

namespace qtllm::toolsinside {

class ToolsInsideTraceRecorder;

class ToolsInsideRuntime
{
public:
    static ToolsInsideRuntime &instance();

    void configureWorkspaceRoot(const QString &workspaceRoot);
    QString workspaceRoot() const;

    void setStoragePolicy(const ToolsInsideStoragePolicy &policy);
    ToolsInsideStoragePolicy storagePolicy() const;

    void setRedactionPolicy(std::shared_ptr<IToolsInsideRedactionPolicy> redactionPolicy);
    const IToolsInsideRedactionPolicy &redactionPolicy() const;

    std::shared_ptr<ToolsInsideRepository> repository() const;
    std::shared_ptr<ToolsInsideArtifactStore> artifactStore() const;
    std::shared_ptr<ToolsInsideQueryService> queryService() const;
    std::shared_ptr<ToolsInsideAdminService> adminService() const;
    std::shared_ptr<ToolsInsideTraceRecorder> recorder() const;

private:
    ToolsInsideRuntime();
    void rebuildPaths();

private:
    QString m_workspaceRoot;
    ToolsInsideStoragePolicy m_storagePolicy;
    std::shared_ptr<IToolsInsideRedactionPolicy> m_redactionPolicy;
    std::shared_ptr<ToolsInsideRepository> m_repository;
    std::shared_ptr<ToolsInsideArtifactStore> m_artifactStore;
    std::shared_ptr<ToolsInsideQueryService> m_queryService;
    std::shared_ptr<ToolsInsideAdminService> m_adminService;
    std::shared_ptr<ToolsInsideTraceRecorder> m_recorder;
};

} // namespace qtllm::toolsinside
