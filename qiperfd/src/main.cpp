#include <QCoreApplication>

#include "qiperfd.h"
#include "../src/comm.h"
#include <jcon/json_rpc_websocket_server.h>
#include "myservice.h"

#if defined (Q_OS_LINUX)&& !defined(Q_OS_ANDROID)
#include <systemd/sd-daemon.h>
#endif

jcon::JsonRpcServer* startServer(QObject* parent,
                                 bool allow_notifications = false)
{
    jcon::JsonRpcServer* rpc_server;
    qDebug() << "Starting WebSocket server";
    rpc_server = new jcon::JsonRpcWebSocketServer(parent);

    if (allow_notifications)
        rpc_server->enableSendNotification(true);

    auto service1 = new MyService;
//    auto service2 = new NotificationService;
    rpc_server->registerServices({ service1 });
//    rpc_server->registerServices({ service1, service2 });
    rpc_server->listen(RPC_PORT);
    return rpc_server;
}

int main(int argc, char *argv[])
{
    int rc;
    QCoreApplication app(argc, argv);
    app.setOrganizationName(QIPERF_ORG);
    app.setOrganizationDomain(QIPERF_DOMAIN);
    app.setApplicationName(QIPERFD_NAME);
    auto server = startServer(nullptr, true);
    QIperfd qiperfd = QIperfd();
#if defined (Q_OS_LINUX)&& !defined(Q_OS_ANDROID)
    sd_notify(0, "READY=1");
#endif
    rc = app.exec();
    delete server;
#if defined (Q_OS_LINUX)&& !defined(Q_OS_ANDROID)
    sd_notify(0, "STOPPING=1");
#endif
    return rc;
}
