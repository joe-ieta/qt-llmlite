#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

namespace qtllm::profile {

struct MemoryPolicy
{
    int maxHistoryMessages = 30;
    int summaryTriggerMessages = 80;
    bool persistSensitiveData = false;
    QStringList persistAllowList;
    QStringList persistBlockList;

    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj.insert(QStringLiteral("maxHistoryMessages"), maxHistoryMessages);
        obj.insert(QStringLiteral("summaryTriggerMessages"), summaryTriggerMessages);
        obj.insert(QStringLiteral("persistSensitiveData"), persistSensitiveData);

        QJsonArray allowList;
        for (const QString &item : persistAllowList) {
            allowList.append(item);
        }
        obj.insert(QStringLiteral("persistAllowList"), allowList);

        QJsonArray blockList;
        for (const QString &item : persistBlockList) {
            blockList.append(item);
        }
        obj.insert(QStringLiteral("persistBlockList"), blockList);

        return obj;
    }

    static MemoryPolicy fromJson(const QJsonObject &obj)
    {
        MemoryPolicy policy;

        if (obj.contains(QStringLiteral("maxHistoryMessages"))) {
            policy.maxHistoryMessages = obj.value(QStringLiteral("maxHistoryMessages")).toInt(policy.maxHistoryMessages);
        }
        if (obj.contains(QStringLiteral("summaryTriggerMessages"))) {
            policy.summaryTriggerMessages = obj.value(QStringLiteral("summaryTriggerMessages")).toInt(policy.summaryTriggerMessages);
        }
        if (obj.contains(QStringLiteral("persistSensitiveData"))) {
            policy.persistSensitiveData = obj.value(QStringLiteral("persistSensitiveData")).toBool(policy.persistSensitiveData);
        }

        const QJsonArray allowList = obj.value(QStringLiteral("persistAllowList")).toArray();
        for (const QJsonValue &value : allowList) {
            const QString item = value.toString();
            if (!item.isEmpty()) {
                policy.persistAllowList.append(item);
            }
        }

        const QJsonArray blockList = obj.value(QStringLiteral("persistBlockList")).toArray();
        for (const QJsonValue &value : blockList) {
            const QString item = value.toString();
            if (!item.isEmpty()) {
                policy.persistBlockList.append(item);
            }
        }

        return policy;
    }
};

} // namespace qtllm::profile
