#pragma once

#include "../runtime/toolruntime_types.h"

#include <QList>
#include <QString>

namespace qtllm::tools::protocol {

class IToolCallProtocolAdapter
{
public:
    virtual ~IToolCallProtocolAdapter() = default;

    virtual QString adapterId() const = 0;

    virtual QList<runtime::ToolCallRequest> parseToolCalls(const QString &assistantText) const = 0;

    virtual QString buildFollowUpPrompt(const QString &assistantText,
                                        const QList<runtime::ToolExecutionResult> &results) const = 0;
};

} // namespace qtllm::tools::protocol
