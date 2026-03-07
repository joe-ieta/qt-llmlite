#pragma once

#include "openaicompatibleprovider.h"

namespace qtllm {

class VllmProvider : public OpenAICompatibleProvider
{
public:
    QString name() const override;
};

} // namespace qtllm
