#include "mainwindow.h"

#include <QApplication>
#include <QObject>
#include "../src/comm.h"
#include "src/mytray.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName(QIPERF_ORG);
    app.setOrganizationDomain(QIPERF_DOMAIN);
    app.setApplicationName(QIPERFC_NAME);

    MyTray * mytray = new MyTray();
    MainWindow w(mytray);

    QObject::connect(mytray, &MyTray::sigShow, &w, &QMainWindow::show);
    QObject::connect(mytray, &MyTray::sigQuit, &w, &QApplication::quit);
    QObject::connect(mytray, &MyTray::sigIconActivated, &w, &MainWindow::onTrayIconActivated);
//    w.show();
    return app.exec();
}
