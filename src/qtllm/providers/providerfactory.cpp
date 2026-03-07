#include "providerfactory.h"

#include "ollamaprovider.h"
#include "openaicompatibleprovider.h"
#include "vllmprovider.h"

#include <QString>

std::unique_ptr<ILLMProvider> ProviderFactory::create(const QString &providerName)
{
    if (providerName == QStringLiteral("ollama")) {
        return std::make_unique<OllamaProvider>();
    }

    if (providerName == QStringLiteral("vllm")) {
        return std::make_unique<VllmProvider>();
    }

    if (providerName == QStringLiteral("openai") ||
        providerName == QStringLiteral("openai-compatible") ||
        providerName == QStringLiteral("sglang")) {
        return std::make_unique<OpenAICompatibleProvider>();
    }

    return nullptr;
}