#pragma once

#include "../domain/documenttranslationtask.h"

#include <functional>
#include <QString>
#include <memory>

namespace pdftranslator::skills {
class SkillRegistry;
class ModelRouter;
}

namespace pdftranslator::pipeline {

struct DocumentWorkflowRunResult
{
    domain::DocumentTranslationTask task;
    QString extractedText;
    QString translatedText;
    QString errorMessage;
};

class DocumentWorkflowController
{
public:
    using ProgressCallback = std::function<void(const domain::DocumentTranslationTask &)>;

    DocumentWorkflowController(const std::shared_ptr<skills::SkillRegistry> &skillRegistry,
                               const std::shared_ptr<skills::ModelRouter> &modelRouter);

    DocumentWorkflowRunResult runSingleDocumentTranslation(const QString &pdfPath,
                                                           const ProgressCallback &progressCallback = ProgressCallback()) const;

private:
    std::shared_ptr<skills::SkillRegistry> m_skillRegistry;
    std::shared_ptr<skills::ModelRouter> m_modelRouter;
};

} // namespace pdftranslator::pipeline
