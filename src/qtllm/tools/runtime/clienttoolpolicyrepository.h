#pragma once

#include "toolruntime_types.h"

#include <optional>

namespace qtllm::tools::runtime {

class ClientToolPolicyRepository
{
public:
    explicit ClientToolPolicyRepository(QString rootDirectory = QStringLiteral(".qtllm/clients"));

    bool savePolicy(const ClientToolPolicy &policy, QString *errorMessage = nullptr) const;
    std::optional<ClientToolPolicy> loadPolicy(const QString &clientId, QString *errorMessage = nullptr) const;

private:
    QString policyPath(const QString &clientId) const;
    bool ensureClientDirectory(const QString &clientId, QString *errorMessage = nullptr) const;

private:
    QString m_rootDirectory;
};

} // namespace qtllm::tools::runtime
