#include <QCoreApplication>

#include "qiperfd.h"
#include "../src/comm.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName(QIPERF_ORG);
    app.setOrganizationDomain(QIPERF_DOMAIN);
    app.setApplicationName(QIPERFD_NAME);
    QIperfd qiperfd = QIperfd();
    return app.exec();
}
