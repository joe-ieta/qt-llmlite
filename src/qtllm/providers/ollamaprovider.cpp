#include "ollamaprovider.h"

namespace qtllm {

QString OllamaProvider::name() const
{
    return QStringLiteral("ollama");
}

} // namespace qtllm
