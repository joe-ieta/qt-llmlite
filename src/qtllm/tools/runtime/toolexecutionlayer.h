#pragma once

#include "toolruntime_types.h"
#include "toolexecutorregistry.h"
#include "toolruntimehooks.h"

#include <QList>

#include <memory>

namespace qtllm::tools::runtime {

class ToolExecutionLayer
{
public:
    ToolExecutionLayer();

    void setRegistry(const std::shared_ptr<ToolExecutorRegistry> &registry);
    void setPolicy(const ToolExecutionPolicy &policy);
    void setHooks(const std::shared_ptr<ToolRuntimeHooks> &hooks);
    void setDryRunFailureMode(bool enabled);

    QList<ToolExecutionResult> executeBatch(const QList<ToolCallRequest> &requests,
                                            const ToolExecutionContext &context,
                                            const ClientToolPolicy &clientPolicy = ClientToolPolicy()) const;

    bool cancelBySession(const QString &clientId, const QString &sessionId) const;

private:
    ToolExecutionResult executeSingle(const ToolCallRequest &request,
                                      const ToolExecutionContext &context,
                                      const ClientToolPolicy &clientPolicy) const;

private:
    std::shared_ptr<ToolExecutorRegistry> m_registry;
    std::shared_ptr<ToolRuntimeHooks> m_hooks;
    ToolExecutionPolicy m_policy;
    bool m_dryRunFailureMode = false;
};

} // namespace qtllm::tools::runtime
