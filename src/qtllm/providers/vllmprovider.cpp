#include "vllmprovider.h"

namespace qtllm {

QString VllmProvider::name() const
{
    return QStringLiteral("vllm");
}

} // namespace qtllm
