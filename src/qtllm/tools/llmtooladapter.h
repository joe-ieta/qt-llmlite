#pragma once

#include "llmtooldefinition.h"

#include <QJsonArray>
#include <QList>
#include <QString>

namespace qtllm::tools {

class ILlmToolAdapter
{
public:
    virtual ~ILlmToolAdapter() = default;

    virtual QJsonArray adaptTools(const QList<LlmToolDefinition> &tools,
                                  const QString &providerName) const = 0;
};

class DefaultLlmToolAdapter : public ILlmToolAdapter
{
public:
    QJsonArray adaptTools(const QList<LlmToolDefinition> &tools,
                          const QString &providerName) const override;
};

} // namespace qtllm::tools
