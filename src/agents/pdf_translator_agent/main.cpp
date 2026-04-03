#include "app/applicationbootstrap.h"

#include <QApplication>
#include <QCoreApplication>
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.network.monitor.warning=false"));
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("CodexDev"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("codexdev.local"));
    QCoreApplication::setApplicationName(QStringLiteral("pdf_translator_agent"));

    pdftranslator::ApplicationBootstrap bootstrap;
    bootstrap.show();

    return app.exec();
}
