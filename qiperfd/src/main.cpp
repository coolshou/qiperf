/*
 * Require run as root
 *
*/
#include <QCoreApplication>
#include <QObject>

#include "qiperfd.h"
#include "../src/comm.h"
#include <jcon/json_rpc_websocket_server.h>
#include "myservice.h"
#if defined(Q_OS_LINUX)
#include "../qt-unix-signals/sigwatch.h"
#endif
#if defined (Q_OS_LINUX)&& !defined(Q_OS_ANDROID)
#include <systemd/sd-daemon.h>
#endif

int isNotRoot()
{ //check if we run as root or administrator
#if defined(Q_OS_LINUX)
    if (geteuid()) {
        printf("%s", "Please run this app as root! (sudo)\n");
        return 1;
    }
#endif

#if defined(Q_OS_WIN32)
    //TODO: check windows administrator
#endif
    return 0;
}
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "DEBUG: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        //fprintf(stderr, "INFO: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        fprintf(stderr, "INFO: %s", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "WARN: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "CRITICAL: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "FATAL: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    }
}
jcon::JsonRpcServer* startServer(QObject* parent,
                                 bool allow_notifications = false, QIperfd* qiperfd=nullptr)
{
    jcon::JsonRpcServer* rpc_server;
    qDebug() << "Starting WebSocket server";
    rpc_server = new jcon::JsonRpcWebSocketServer(parent);

    if (allow_notifications)
        rpc_server->enableSendNotification(true);

    auto service1 = new MyService(qiperfd);
    QObject::connect(service1, SIGNAL(sig_setManagerInterface(QString)), qiperfd, SLOT(setManagerInterface(QString)));
//    auto service2 = new NotificationService;
    rpc_server->registerServices({ service1 });
//    rpc_server->registerServices({ service1, service2 });
    rpc_server->listen(RPC_PORT);
    return rpc_server;
}

int main(int argc, char *argv[])
{
    int rc;
    rc = isNotRoot();
    if (rc == 0){
        //TODO: log file
        qInstallMessageHandler(myMessageOutput);

        QCoreApplication app(argc, argv);
    #if defined(Q_OS_LINUX)
        UnixSignalWatcher sigwatch;
        sigwatch.watchForSignal(SIGINT);
    #endif
        app.setOrganizationName(QIPERF_ORG);
        app.setOrganizationDomain(QIPERF_DOMAIN);
        app.setApplicationName(QIPERFD_NAME);

        QIperfd qiperfd = QIperfd();
    //    QObject::connect(&app, SIGNAL(aboutToQuit()), &qiperfd , SLOT(onQuit()));
    #if defined(Q_OS_LINUX)
        QObject::connect(&sigwatch, SIGNAL(unixSignal(int)), &qiperfd, SLOT(onQuit()));
    #endif
        auto server = startServer(nullptr, true, &qiperfd);

    #if defined (Q_OS_LINUX)&& !defined(Q_OS_ANDROID)
        sd_notify(0, "READY=1");
    #endif
        rc = app.exec();
        delete server;
    #if defined (Q_OS_LINUX)&& !defined(Q_OS_ANDROID)
        sd_notify(0, "STOPPING=1");
    #endif
    }
    return rc;
}
