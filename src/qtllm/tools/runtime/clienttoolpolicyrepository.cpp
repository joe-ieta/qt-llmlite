#include "clienttoolpolicyrepository.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>

namespace qtllm::tools::runtime {

ClientToolPolicyRepository::ClientToolPolicyRepository(QString rootDirectory)
    : m_rootDirectory(std::move(rootDirectory))
{
}

bool ClientToolPolicyRepository::savePolicy(const ClientToolPolicy &policy, QString *errorMessage) const
{
    const QString clientId = policy.clientId.trimmed();
    if (clientId.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("ClientToolPolicy clientId is empty");
        }
        return false;
    }

    if (!ensureClientDirectory(clientId, errorMessage)) {
        return false;
    }

    QSaveFile file(policyPath(clientId));
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open client tool policy for writing: ") + file.errorString();
        }
        return false;
    }

    QJsonObject root;
    root.insert(QStringLiteral("clientId"), clientId);
    root.insert(QStringLiteral("mode"), policy.mode);
    root.insert(QStringLiteral("maxToolsPerTurn"), policy.maxToolsPerTurn);

    QJsonArray allow;
    for (const QString &id : policy.allowIds) {
        allow.append(id);
    }
    root.insert(QStringLiteral("allowIds"), allow);

    QJsonArray deny;
    for (const QString &id : policy.denyIds) {
        deny.append(id);
    }
    root.insert(QStringLiteral("denyIds"), deny);

    const QJsonDocument doc(root);
    if (file.write(doc.toJson(QJsonDocument::Indented)) < 0) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to write client tool policy: ") + file.errorString();
        }
        return false;
    }

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to commit client tool policy: ") + file.errorString();
        }
        return false;
    }

    return true;
}

std::optional<ClientToolPolicy> ClientToolPolicyRepository::loadPolicy(const QString &clientId,
                                                                        QString *errorMessage) const
{
    const QString resolvedClientId = clientId.trimmed();
    if (resolvedClientId.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("ClientToolPolicy clientId is empty");
        }
        return std::nullopt;
    }

    QFile file(policyPath(resolvedClientId));
    if (!file.exists()) {
        return std::nullopt;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Failed to open client tool policy for reading: ") + file.errorString();
        }
        return std::nullopt;
    }

    const QByteArray payload = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Client tool policy JSON parse error: ") + parseError.errorString();
        }
        return std::nullopt;
    }

    ClientToolPolicy policy;
    const QJsonObject root = doc.object();
    policy.clientId = root.value(QStringLiteral("clientId")).toString();
    policy.mode = root.value(QStringLiteral("mode")).toString(policy.mode);
    policy.maxToolsPerTurn = root.value(QStringLiteral("maxToolsPerTurn")).toInt(policy.maxToolsPerTurn);

    const QJsonArray allow = root.value(QStringLiteral("allowIds")).toArray();
    for (const QJsonValue &value : allow) {
        const QString id = value.toString();
        if (!id.isEmpty()) {
            policy.allowIds.append(id);
        }
    }

    const QJsonArray deny = root.value(QStringLiteral("denyIds")).toArray();
    for (const QJsonValue &value : deny) {
        const QString id = value.toString();
        if (!id.isEmpty()) {
            policy.denyIds.append(id);
        }
    }

    if (policy.clientId.isEmpty()) {
        policy.clientId = resolvedClientId;
    }

    return policy;
}

QString ClientToolPolicyRepository::policyPath(const QString &clientId) const
{
    return QDir(m_rootDirectory).filePath(clientId + QStringLiteral("/tool_policy.json"));
}

bool ClientToolPolicyRepository::ensureClientDirectory(const QString &clientId,
                                                       QString *errorMessage) const
{
    QDir dir(QDir(m_rootDirectory).filePath(clientId));
    if (dir.exists()) {
        return true;
    }

    if (dir.mkpath(QStringLiteral("."))) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = QStringLiteral("Failed to create client policy directory for: ") + clientId;
    }
    return false;
}

} // namespace qtllm::tools::runtime
