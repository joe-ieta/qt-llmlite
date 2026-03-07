#pragma once

#include "openaicompatibleprovider.h"

namespace qtllm {

class OllamaProvider : public OpenAICompatibleProvider
{
public:
    QString name() const override;
};

} // namespace qtllm
