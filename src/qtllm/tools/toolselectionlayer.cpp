#include "toolselectionlayer.h"

#include <QtGlobal>

namespace qtllm::tools {

namespace {

bool hasMatchingTag(const QStringList &tags, const QString &content)
{
    const QString loweredContent = content.toLower();
    for (const QString &tag : tags) {
        if (!tag.trimmed().isEmpty() && loweredContent.contains(tag.toLower())) {
            return true;
        }
    }
    return false;
}

} // namespace

QList<LlmToolDefinition> ToolSelectionLayer::selectTools(const profile::ClientProfile &profile,
                                                         const QVector<LlmMessage> &history,
                                                         const QList<LlmToolDefinition> &availableTools,
                                                         int maxTools) const
{
    QList<LlmToolDefinition> selected;
    if (availableTools.isEmpty() || maxTools <= 0) {
        return selected;
    }

    QString lastUserContent;
    for (int i = history.size() - 1; i >= 0; --i) {
        if (history.at(i).role == QStringLiteral("user")) {
            lastUserContent = history.at(i).content;
            break;
        }
    }

    for (const LlmToolDefinition &tool : availableTools) {
        if (!tool.enabled) {
            continue;
        }

        bool match = false;

        if (!profile.capabilityTags.isEmpty() && hasMatchingTag(profile.capabilityTags, tool.description)) {
            match = true;
        }

        if (!match && !lastUserContent.trimmed().isEmpty() && hasMatchingTag(tool.capabilityTags, lastUserContent)) {
            match = true;
        }

        if (!match && selected.isEmpty()) {
            // Fallback: ensure at least one candidate when nothing matches.
            match = true;
        }

        if (match) {
            selected.append(tool);
        }

        if (selected.size() >= maxTools) {
            break;
        }
    }

    return selected;
}

} // namespace qtllm::tools
