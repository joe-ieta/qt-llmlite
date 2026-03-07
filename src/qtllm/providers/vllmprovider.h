#pragma once

#include "openaicompatibleprovider.h"

class VllmProvider : public OpenAICompatibleProvider
{
public:
    QString name() const override;
};
