#include "toolstudiowindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("qtllm"));
    QApplication::setApplicationName(QStringLiteral("toolstudio"));

    ToolStudioWindow window;
    window.show();
    return app.exec();
}
