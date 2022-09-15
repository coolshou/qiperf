#include "mainwindow.h"

#include <QApplication>

#include "qiperf.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName(APP_ORG);
    a.setOrganizationDomain(APP_DOMAIN);
    a.setApplicationName(APP_NAME);
    MainWindow w;
    w.show();
    return a.exec();
}
