#include "mainwindow.h"

#include <QApplication>
#include "../src/comm.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName(QIPERF_ORG);
    app.setOrganizationDomain(QIPERF_DOMAIN);
    app.setApplicationName(QIPERFC_NAME);
    MainWindow w;
    w.show();
    return app.exec();
}
