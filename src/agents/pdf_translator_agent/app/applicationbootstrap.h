#pragma once

#include <memory>

class QWidget;

namespace pdftranslator {

namespace skills {
class SkillRegistry;
class ModelRouter;
namespace mcp {
class McpGateway;
}
}

class ApplicationBootstrap
{
public:
    ApplicationBootstrap();
    ~ApplicationBootstrap();

    void show();

private:
    void initializeSkills();

private:
    std::shared_ptr<skills::SkillRegistry> m_skillRegistry;
    std::shared_ptr<skills::ModelRouter> m_modelRouter;
    std::shared_ptr<skills::mcp::McpGateway> m_mcpGateway;
    QWidget *m_mainWindow = nullptr;
};

} // namespace pdftranslator
