#pragma once

#include <memory>

class ILLMProvider;
class QString;

class ProviderFactory
{
public:
    static std::unique_ptr<ILLMProvider> create(const QString &providerName);
};