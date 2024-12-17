
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
#include <QMessageBox>
#include <qlogging.h>

static QTextStream output_ts;
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QDateTime t = QDateTime::currentDateTime();
    output_ts << "[" + t.toString("yyyy-MM-dd hh:mm:ss.zzz") + "] ";
    //qDebug() << "myMessageOutput: " << msg << Qt::endl;
    const char *file = context.file ? context.file : "";
    //    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        output_ts << QString("DEBUG: %1 (%2:%3)").arg(msg, file).arg(context.line) << Qt::endl;
        break;
    case QtInfoMsg:
        output_ts << QString("INFO: %1 (%2:%3)").arg(msg, file).arg(context.line) << Qt::endl;
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

    QString logfilePath = tmp + QDir::separator() + QIPERFC_NAME + QDir::separator();
    QDir dir(logfilePath);
    if (!dir.exists())
        dir.mkpath(".");
    QString logfile = logfilePath + QIPERFC_NAME + ".log";
    qDebug() << "logfile:  " << logfile;
    if (QFile::exists(logfile)){
        // check log file exist, backup it
        QFileInfo finfo(logfile);
        QDateTime oldtime =  finfo.fileTime(QFileDevice::FileModificationTime);
        qDebug() << "logfile ModificationTime: "  << oldtime;
        QString baklogfile =  logfilePath + QIPERFC_NAME + "_" + oldtime.toString("yyyy-MM-dd_hhmmss.zzz")+ ".log";
        QFile::rename(logfile, baklogfile);
    }

    qInstallMessageHandler(myMessageOutput);
    QApplication app(argc, argv);
    QFile outFile(logfile);
    if (! outFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        QString s =  "open file " + logfile + " Fail" ;
        qDebug() << s << Qt::endl;
        QMessageBox::warning(nullptr, "ERROR", s);
        return -1;
    } else {
        output_ts.setDevice(&outFile);
    }

#if defined(Q_OS_LINUX) && TEST_SIGWATCH
    UnixSignalWatcher sigwatch;
    sigwatch.watchForSignal(SIGINT);
#endif

    app.setOrganizationName(QIPERF_ORG);
    app.setOrganizationDomain(QIPERF_DOMAIN);
    app.setApplicationName(QIPERFC_NAME);
    QIperfC main(logfilePath);
#if defined(Q_OS_LINUX) && TEST_SIGWATCH
    QObject::connect(&sigwatch, SIGNAL(unixSignal(int)), &main, SLOT(onQuit()));
#endif

    if (argc >=2) {
        QFileInfo fi(argv[1]);
        if (fi.suffix().compare(QIPERF_EXT)==0){
            main.load(argv[1]);
        }else{
            qDebug() << "unknown file ext: " << argv[1] ;
        }
    }
    main.show();
    rc = app.exec();

    return rc;
}
