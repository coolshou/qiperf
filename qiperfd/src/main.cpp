/*
 * Require run as root
 *
*/

#include <qglobal.h>
#if defined(Q_OS_LINUX)
#include <unistd.h>
#include <sys/types.h>
#endif

#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QMessageLogContext>
#include <qlogging.h>

#include "qiperfd.h"
#include "../src/comm.h"
#include <jcon/json_rpc_websocket_server.h>
#if (USE_JSONRPC==1)
#include "myservice.h"
#endif
#include <QCtrlSignals>

//#include "../src/mylog.h"

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


#if (USE_JSONRPC==1)
jcon::JsonRpcServer* startServer(QObject* parent,
                                 bool allow_notifications = false, QIperfd* qiperfd=nullptr)
{
    jcon::JsonRpcServer* rpc_server;
    qDebug() << "Starting JsonRpc WebSocket server";
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
#endif
static QTextStream output_ts;
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    qDebug() << "myMessageOutput: " << msg << Qt::endl;
    const char *file = context.file ? context.file : "";
    //    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        output_ts << QString("DEBUG: %1 (%2:%3)").arg(msg, file).arg(context.line) << Qt::endl;
        break;
    case QtInfoMsg:
        output_ts << QString("INFO: %1 ").arg(msg) << Qt::endl;
        break;
    case QtWarningMsg:
        output_ts << QString("WARN: %1 (%2:%3)").arg(msg, file).arg(context.line) << Qt::endl;
        break;
    case QtCriticalMsg:
        output_ts << QString("CRITICAL: %1 (%2:%3)").arg(msg, file).arg(context.line) << Qt::endl;
        break;
    case QtFatalMsg:
        output_ts << QString("FATAL: %1 (%2:%3)").arg(msg, file).arg(context.line) << Qt::endl;
        break;
    }
}

int main(int argc, char *argv[])
{
    int rc;
    rc = isNotRoot();
    if (rc == 0){
        //log file
        QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

        QString logfilePath = tmp + "/qiperf/";
        QDir dir(logfilePath);
        if (!dir.exists())
            dir.mkpath(".");
        QString logfile = logfilePath + "qiperfd.log";
        // TODO: check log file exist, backup it
        qDebug() << "logfile: " << logfile <<  Qt::endl;
        QFile outFile(logfile);
        if (! outFile.open(QIODevice::WriteOnly | QIODevice::Append)){
            qDebug() << "open file " << logfile << " Fail" << Qt::endl;
        } else {
            output_ts.setDevice(&outFile);
        }
        qInstallMessageHandler(myMessageOutput);

        QCoreApplication app(argc, argv);
        // handle ctrl+c
        auto handler = QCtrlSignalHandler::instance();
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, qApp, [](){
                qDebug() << "App about to quit!";
                QThread::sleep(1);
            }, Qt::DirectConnection);
        handler->setAutoQuitActive(true);

        app.setOrganizationName(QIPERF_ORG);
        app.setOrganizationDomain(QIPERF_DOMAIN);
        app.setApplicationName(QIPERFD_NAME);

        PipeServer *m_pserver = new PipeServer(QIPERFD_NAME, app.applicationPid(), nullptr);
        if (m_pserver->init())
        {
            printf("%s", "Another server is running! Get args and send to the server\n");
            //trying to get the arguments into a list
            QStringList cmdline_args = app.arguments();
            cmdline_args.removeFirst();
            m_pserver->sendARGS(cmdline_args);
            return -1;
        } else
        {
            QIperfd qiperfd = QIperfd(m_pserver);
//            connect(m_pserver, SIGNAL(newMessage(int, QString)), this, SLOT(onPipeMessage(int, QString)));
        }

#if (USE_JSONRPC==1)
        auto server = startServer(nullptr, true, &qiperfd);
#endif
        rc = app.exec();
#if (USE_JSONRPC==1)
        delete server;
#endif
    }
    return rc;
}
