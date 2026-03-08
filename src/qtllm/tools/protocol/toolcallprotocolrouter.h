#pragma once

#include "itoolcallprotocoladapter.h"

#include <memory>

namespace qtllm::tools::protocol {

class ToolCallProtocolRouter
{
public:
    // Route by model vendor/family first; provider is only a fallback signal.
    std::shared_ptr<IToolCallProtocolAdapter> route(const QString &modelName,
                                                    const QString &modelVendor = QString(),
                                                    const QString &providerName = QString()) const;
};

} // namespace qtllm::tools::protocol
