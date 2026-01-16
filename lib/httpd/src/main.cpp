#include <qglobal.h>
#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    int rc;
    QApplication app(argc, argv);

    MainWindow main;
    main.show();
    rc = app.exec();

    return rc;
}
