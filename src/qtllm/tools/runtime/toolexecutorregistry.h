#pragma once

#include "itoolexecutor.h"

#include <QHash>
#include <QString>

#include <memory>

namespace qtllm::tools::runtime {

class ToolExecutorRegistry
{
public:
    bool registerExecutor(const std::shared_ptr<IToolExecutor> &executor);
    bool unregisterExecutor(const QString &toolId);
    std::shared_ptr<IToolExecutor> find(const QString &toolId) const;
    void clear();

private:
    QHash<QString, std::shared_ptr<IToolExecutor>> m_executors;
};

} // namespace qtllm::tools::runtime
