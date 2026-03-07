#pragma once

#include "openaicompatibleprovider.h"

class OllamaProvider : public OpenAICompatibleProvider
{
public:
    QString name() const override;
};
