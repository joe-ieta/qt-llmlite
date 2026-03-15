#pragma once

#include "logtypes.h"

namespace qtllm::logging {

class ILogSink
{
public:
    virtual ~ILogSink() = default;
    virtual void write(const LogEvent &event) = 0;
};

} // namespace qtllm::logging
