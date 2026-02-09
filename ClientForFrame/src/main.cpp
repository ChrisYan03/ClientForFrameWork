#include "ClientMainWidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ClientMainWidget w;
    w.show();
    w.raise();
    w.DemoInit();
    QObject::connect(&app, &QApplication::aboutToQuit, [&w](){
        qDebug() << "Performing cleanup...";
        w.DemoQuit();
    });
    w.DemoRun();
    return app.exec();
}
