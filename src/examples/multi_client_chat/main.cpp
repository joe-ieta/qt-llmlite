#include "multiclientwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MultiClientWindow window;
    window.show();

    return app.exec();
}
