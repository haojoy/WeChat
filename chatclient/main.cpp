#include "Controls/kernel.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Kernel* kernel = new Kernel();

    QObject::connect(&app, &QApplication::aboutToQuit, [kernel]() {
        delete kernel;
    });
    return app.exec();
}

