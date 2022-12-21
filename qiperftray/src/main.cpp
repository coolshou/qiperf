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

    MainWindow w;
    MyTray * mytray = new MyTray();
    QObject::connect(mytray, &MyTray::sigShow, &w, &QMainWindow::show);

//    w.show();
    return app.exec();
}
