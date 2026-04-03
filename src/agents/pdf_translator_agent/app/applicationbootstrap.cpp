#include "applicationbootstrap.h"

#include "../pipeline/documentworkflowcontroller.h"
#include "../skills/builtin/chunktranslateskill.h"
#include "../skills/builtin/documentassembleskill.h"
#include "../skills/builtin/languagedetectskill.h"
#include "../skills/builtin/pdftextextractskill.h"
#include "../skills/builtin/termbaseresolveskill.h"
#include "../skills/mcp/mcpgateway.h"
#include "../skills/core/modelrouter.h"
#include "../skills/core/skillregistry.h"
#include "../ui/mainwindow.h"

namespace pdftranslator {

ApplicationBootstrap::ApplicationBootstrap()
    : m_skillRegistry(std::make_shared<skills::SkillRegistry>())
    , m_modelRouter(std::make_shared<skills::ModelRouter>())
    , m_mcpGateway(std::make_shared<skills::mcp::McpGateway>())
{
    initializeSkills();
    m_mainWindow = new MainWindow(m_skillRegistry, m_modelRouter, m_mcpGateway);
}

ApplicationBootstrap::~ApplicationBootstrap()
{
    delete m_mainWindow;
}

void ApplicationBootstrap::show()
{
    if (m_mainWindow) {
        m_mainWindow->show();
    }
}

void ApplicationBootstrap::initializeSkills()
{
    m_skillRegistry->registerSkill(std::make_shared<skills::LanguageDetectSkill>(m_modelRouter));
    m_skillRegistry->registerSkill(std::make_shared<skills::PdfTextExtractSkill>(m_modelRouter, m_mcpGateway));
    m_skillRegistry->registerSkill(std::make_shared<skills::TermBaseResolveSkill>(m_modelRouter, m_mcpGateway));
    m_skillRegistry->registerSkill(std::make_shared<skills::ChunkTranslateSkill>(m_modelRouter));
    m_skillRegistry->registerSkill(std::make_shared<skills::DocumentAssembleSkill>());
}

} // namespace pdftranslator
