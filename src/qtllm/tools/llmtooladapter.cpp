#include "llmtooladapter.h"

#include <QJsonObject>

namespace qtllm::tools {

QJsonArray DefaultLlmToolAdapter::adaptTools(const QList<LlmToolDefinition> &tools,
                                             const QString &providerName) const
{
    Q_UNUSED(providerName)

    QJsonArray result;
    for (const LlmToolDefinition &tool : tools) {
        if (!tool.enabled) {
            continue;
        }

        QJsonObject function;
        function.insert(QStringLiteral("name"), tool.invocationName);
        function.insert(QStringLiteral("description"), tool.description);
        function.insert(QStringLiteral("parameters"), tool.inputSchema);

        QJsonObject descriptor;
        descriptor.insert(QStringLiteral("type"), QStringLiteral("function"));
        descriptor.insert(QStringLiteral("function"), function);
        result.append(descriptor);
    }

    return result;
}

} // namespace qtllm::tools
