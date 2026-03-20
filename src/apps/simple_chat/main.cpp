#include "chatwindow.h"

#include <QApplication>
#include <QLoggingCategory>

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.network.monitor.warning=false"));
    QApplication app(argc, argv);

    ChatWindow window;
    window.show();

    return app.exec();
}
