#pragma once

#include "itoolcallprotocoladapter.h"

namespace qtllm::tools::protocol {

class OpenAIToolCallProtocolAdapter : public IToolCallProtocolAdapter
{
public:
    QString adapterId() const override;
    QList<runtime::ToolCallRequest> parseToolCalls(const QString &assistantText) const override;
    QString buildFollowUpPrompt(const QString &assistantText,
                                const QList<runtime::ToolExecutionResult> &results) const override;
};

class OllamaToolCallProtocolAdapter : public IToolCallProtocolAdapter
{
public:
    QString adapterId() const override;
    QList<runtime::ToolCallRequest> parseToolCalls(const QString &assistantText) const override;
    QString buildFollowUpPrompt(const QString &assistantText,
                                const QList<runtime::ToolExecutionResult> &results) const override;
};

class VllmToolCallProtocolAdapter : public IToolCallProtocolAdapter
{
public:
    QString adapterId() const override;
    QList<runtime::ToolCallRequest> parseToolCalls(const QString &assistantText) const override;
    QString buildFollowUpPrompt(const QString &assistantText,
                                const QList<runtime::ToolExecutionResult> &results) const override;
};

class GenericToolCallProtocolAdapter : public IToolCallProtocolAdapter
{
public:
    QString adapterId() const override;
    QList<runtime::ToolCallRequest> parseToolCalls(const QString &assistantText) const override;
    QString buildFollowUpPrompt(const QString &assistantText,
                                const QList<runtime::ToolExecutionResult> &results) const override;
};

} // namespace qtllm::tools::protocol
