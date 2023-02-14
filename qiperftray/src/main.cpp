#include "qiperftray.h"

#include <QApplication>
#include <QObject>
#include "../src/comm.h"
#include "src/mytray.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName(QIPERF_ORG);
    app.setOrganizationDomain(QIPERF_DOMAIN);
    app.setApplicationName(QIPERFTRAY_NAME);

    MyTray * mytray = new MyTray();
    QIperfTray w(mytray);
    //w.setWindowFlags( Qt::WindowTitleHint |  Qt::WindowMinimizeButtonHint | Qt::WindowSystemMenuHint);


    QObject::connect(mytray, &MyTray::sigShow, &w, &QIperfTray::show);
    QObject::connect(mytray, &MyTray::sigQuit, &w, &QApplication::quit);
    QObject::connect(mytray, &MyTray::sigIconActivated, &w, &QIperfTray::onTrayIconActivated);
//    w.show();
    return app.exec();
}
