#pragma once

#include "../domain/documenttranslationtask.h"

#include <optional>

namespace pdftranslator::storage {

class ManifestRepository
{
public:
    bool save(const domain::DocumentTranslationTask &task, QString *errorMessage = nullptr) const;
    std::optional<domain::DocumentTranslationTask> load(const QString &manifestPath,
                                                        QString *errorMessage = nullptr) const;
};

} // namespace pdftranslator::storage
