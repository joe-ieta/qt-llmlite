#pragma once

#include "itoolexecutor.h"

namespace qtllm::tools::runtime {

class CurrentTimeToolExecutor : public IToolExecutor
{
public:
    QString toolId() const override;
    ToolExecutionResult execute(const ToolCallRequest &request,
                                const ToolExecutionContext &context) override;
};

class CurrentWeatherToolExecutor : public IToolExecutor
{
public:
    QString toolId() const override;
    ToolExecutionResult execute(const ToolCallRequest &request,
                                const ToolExecutionContext &context) override;
};

} // namespace qtllm::tools::runtime
