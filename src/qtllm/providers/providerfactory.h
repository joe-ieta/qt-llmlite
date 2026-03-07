#pragma once

#include <memory>

class QString;

namespace qtllm {

class ILLMProvider;

class ProviderFactory
{
public:
    static std::unique_ptr<ILLMProvider> create(const QString &providerName);
};

} // namespace qtllm
