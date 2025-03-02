#include "ClientMainWidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientMainWidget w;
    w.show();
    return a.exec();
}
