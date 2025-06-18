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
#include <QFileInfo>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QMessageLogContext>
#include <QMessageBox>
#include <qlogging.h>
#include <stdio.h>

#include "qiperfd.h"
#include "../src/comm.h"
#if defined(Q_OS_WIN32)
#include <Objbase.h> // For CoInitializeEx and CoUninitialize
#endif

#if defined(Q_OS_LINUX)
#include "../QCtrlSignals/src/QCtrlSignals"
#endif
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

static QTextStream output_ts;
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString endl = "\n";
    QDateTime t = QDateTime::currentDateTime();
    output_ts << "[" + t.toString(MYTIMESTEMP) + "] ";

    const char *file = context.file ? context.file : "";
    QString line ="";
    if (file){
        line = QString("(%1:%2)").arg(file, QString::number(context.line));
    }
    //    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        output_ts << QString("DEBUG: %1 %2").arg(msg, line) << endl;
        break;
    case QtInfoMsg:
        output_ts << QString("INFO: %1 %2").arg(msg, line) << endl;
        break;
    case QtWarningMsg:
        output_ts << QString("WARN: %1 %2").arg(msg, line) << endl;
        break;
    case QtCriticalMsg:
        output_ts << QString("CRITICAL: %1 %2").arg(msg, line) << endl;
        break;
    case QtFatalMsg:
        output_ts << QString("FATAL: %1 %2").arg(msg, line) << endl;
        break;
    default:
        QString m = msg + " " + line;
        printf("%s\n", m.toStdString().c_str());
        fflush(stdout);
        break;
    }
    output_ts.flush(); //empty all data from its write buffer into the device
}

int main(int argc, char *argv[])
{
    int rc;
    rc = isNotRoot();
    if (rc == 0){
        //log file
        QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

        QString logfilePath = tmp + QDir::separator() + QIPERF_NAME + QDir::separator();
        QDir dir(logfilePath);
        if (!dir.exists())
            dir.mkpath(".");
        QString logfile = logfilePath + QIPERFD_NAME + ".log";
        qDebug() << "logfile: " << logfile;
        if (QFile::exists(logfile)){
            // check log file exist, backup it
            QFileInfo finfo(logfile);
            QDateTime oldtime =  finfo.fileTime(QFileDevice::FileModificationTime);
            qDebug() << "logfile ModificationTime: "  << oldtime;
            QString baklogfile =  logfilePath + QIPERFD_NAME + "_" + oldtime.toString(DATETIME_NOW_FORMAT)+ ".log";
            QFile::rename(logfile, baklogfile);
        }
        QFile outFile(logfile);
        if (! outFile.open(QIODevice::WriteOnly | QIODevice::Append)){
            qDebug() << "open file " << logfile << " Fail";
        } else {
            output_ts.setDevice(&outFile);
            //output_ts = new QTextStream(&outFile);
        }
        qInstallMessageHandler(myMessageOutput);

        QCoreApplication app(argc, argv);
        // handle ctrl+c
    #if defined(Q_OS_LINUX)
        auto handler = QCtrlSignalHandler::instance();
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, qApp, [](){
                qDebug() << "App about to quit!";
                QThread::sleep(1);
            }, Qt::DirectConnection);
        handler->setAutoQuitActive(true);
    #endif
        app.setOrganizationName(QIPERF_ORG);
        app.setOrganizationDomain(QIPERF_DOMAIN);
        app.setApplicationName(QIPERFD_NAME);
    #if defined(Q_OS_WIN32)
        // Initialize COM for the main thread here, once at startup
        // This is typically called once per thread that uses COM.
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // Or COINIT_MULTITHREADED
        if (FAILED(hr)) {
            QMessageBox::critical(nullptr, "COM Initialization Error",
                                  QString("Failed to initialize COM library: 0x%1").arg(hr, 8, 16, QChar('0').toUpper()));
            return 1;
        }
    #endif
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
            QIperfd *qiperfd = new QIperfd(m_pserver);
            QMetaObject::Connection rc = QObject::connect(m_pserver, &PipeServer::pipeMessage, qiperfd, &QIperfd::onPipeMessage);
            if (!rc){
                qDebug() << "connect pipeMessage fail";
            }
        }
        rc = app.exec();
    }
#if defined(Q_OS_WIN32)
    // Uninitialize COM when the application exits
    CoUninitialize();
#endif
    return rc;
}
