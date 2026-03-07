#pragma once

#include "memorypolicy.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace qtllm::profile {

struct ClientProfile
{
    QString displayName;
    QString systemPrompt;
    QString persona;
    QString thinkingStyle;
    QStringList skillIds;
    QStringList preferenceTags;
    QStringList capabilityTags;
    MemoryPolicy memoryPolicy;

    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj.insert(QStringLiteral("displayName"), displayName);
        obj.insert(QStringLiteral("systemPrompt"), systemPrompt);
        obj.insert(QStringLiteral("persona"), persona);
        obj.insert(QStringLiteral("thinkingStyle"), thinkingStyle);

        QJsonArray skills;
        for (const QString &value : skillIds) {
            skills.append(value);
        }
        obj.insert(QStringLiteral("skillIds"), skills);

        QJsonArray preferences;
        for (const QString &value : preferenceTags) {
            preferences.append(value);
        }
        obj.insert(QStringLiteral("preferenceTags"), preferences);

        QJsonArray capabilities;
        for (const QString &value : capabilityTags) {
            capabilities.append(value);
        }
        obj.insert(QStringLiteral("capabilityTags"), capabilities);

        obj.insert(QStringLiteral("memoryPolicy"), memoryPolicy.toJson());
        return obj;
    }

    static ClientProfile fromJson(const QJsonObject &obj)
    {
        ClientProfile profile;

        profile.displayName = obj.value(QStringLiteral("displayName")).toString();
        profile.systemPrompt = obj.value(QStringLiteral("systemPrompt")).toString();
        profile.persona = obj.value(QStringLiteral("persona")).toString();
        profile.thinkingStyle = obj.value(QStringLiteral("thinkingStyle")).toString();

        const QJsonArray skills = obj.value(QStringLiteral("skillIds")).toArray();
        for (const QJsonValue &value : skills) {
            const QString item = value.toString();
            if (!item.isEmpty()) {
                profile.skillIds.append(item);
            }
        }

        const QJsonArray preferences = obj.value(QStringLiteral("preferenceTags")).toArray();
        for (const QJsonValue &value : preferences) {
            const QString item = value.toString();
            if (!item.isEmpty()) {
                profile.preferenceTags.append(item);
            }
        }

        const QJsonArray capabilities = obj.value(QStringLiteral("capabilityTags")).toArray();
        for (const QJsonValue &value : capabilities) {
            const QString item = value.toString();
            if (!item.isEmpty()) {
                profile.capabilityTags.append(item);
            }
        }

        const QJsonObject memoryPolicyObject = obj.value(QStringLiteral("memoryPolicy")).toObject();
        profile.memoryPolicy = MemoryPolicy::fromJson(memoryPolicyObject);

        return profile;
    }
};

} // namespace qtllm::profile
