#include "providerfactory.h"

#include "ollamaprovider.h"
#include "openaicompatibleprovider.h"
#include "vllmprovider.h"

#include <QString>

namespace qtllm {

std::unique_ptr<ILLMProvider> ProviderFactory::create(const QString &providerName)
{
    const QString provider = providerName.trimmed().toLower();

    if (provider == QStringLiteral("ollama")) {
        return std::make_unique<OllamaProvider>();
    }

    if (provider == QStringLiteral("vllm")) {
        return std::make_unique<VllmProvider>();
    }

    if (provider == QStringLiteral("openai")
        || provider == QStringLiteral("openai-compatible")
        || provider == QStringLiteral("sglang")
        || provider == QStringLiteral("anthropic")
        || provider == QStringLiteral("google")
        || provider == QStringLiteral("gemini")
        || provider == QStringLiteral("deepseek")
        || provider == QStringLiteral("qwen")
        || provider == QStringLiteral("glm")
        || provider == QStringLiteral("zhipu")) {
        return std::make_unique<OpenAICompatibleProvider>();
    }

    return nullptr;
}

} // namespace qtllm
