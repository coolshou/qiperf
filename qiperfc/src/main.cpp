
#include <qglobal.h>
#define TEST_SIGWATCH 0
#if defined(Q_OS_LINUX) && TEST_SIGWATCH
#include "../qt-unix-signals/sigwatch.h"
#endif

#include <QApplication>
#include "qiperfc.h"
#include "../src/comm.h"
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QMessageLogContext>
#include <qlogging.h>

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
    //log file
    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    QString logfilePath = tmp + "/qiperf/";
    QDir dir(logfilePath);
    if (!dir.exists())
        dir.mkpath(".");
    QString logfile = logfilePath + "qiperfc.log";
    // TODO: check log file exist, backup it
    QFile outFile(logfile);
    if (! outFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        qDebug() << "open file " << logfile << " Fail" << Qt::endl;
    } else {
        output_ts.setDevice(&outFile);
    }
    qInstallMessageHandler(myMessageOutput);
    QApplication app(argc, argv);
#if defined(Q_OS_LINUX) && TEST_SIGWATCH
    UnixSignalWatcher sigwatch;
    sigwatch.watchForSignal(SIGINT);
#endif

    app.setOrganizationName(QIPERF_ORG);
    app.setOrganizationDomain(QIPERF_DOMAIN);
    app.setApplicationName(QIPERFC_NAME);
    QIperfC main;
#if defined(Q_OS_LINUX) && TEST_SIGWATCH
    QObject::connect(&sigwatch, SIGNAL(unixSignal(int)), &main, SLOT(onQuit()));
#endif

    main.show();
    rc = app.exec();

    return rc;
}
