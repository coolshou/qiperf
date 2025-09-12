
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
#include <QStyleFactory>
#include <QMutex>

#include <qlogging.h>
static QFile logFile;
static QTextStream output_ts;
static QMutex logMutex;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&logMutex);  // Lock for thread safety
    if (!logFile.isOpen())
        return;

    QString endl = "\n";
    QDateTime t = QDateTime::currentDateTime();
    output_ts << "[" + t.toString(MYTIMESTEMP) + "] ";
    const char *file = context.file ? context.file : "";
    QString line ="";
    if (*file) {
        line = QString("(%1:%2)").arg(file, QString::number(context.line));
    }
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
        QString m = msg + " "+ line;
        printf("%s\n", m.toStdString().c_str());
        fflush(stdout);
        break;
    }
    output_ts.flush(); //empty all data from its write buffer into the device
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
    QString logfilename = logfilePath + QIPERFC_NAME + ".log";
    qDebug() << "logfile:  " << logfilename;
    if (QFile::exists(logfilename)){
        // check log file exist, backup it
        QFileInfo finfo(logfilename);
        QDateTime oldtime =  finfo.fileTime(QFileDevice::FileModificationTime);
        qDebug() << "logfile ModificationTime: "  << oldtime;
        QString baklogfile =  logfilePath + QIPERFC_NAME + "_" + oldtime.toString(DATETIME_NOW_FORMAT)+ ".log";
        QFile::rename(logfilename, baklogfile);
    }
    logFile.setFileName(logfilename);
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        output_ts.setDevice(&logFile);
        qInstallMessageHandler(myMessageOutput);
    }

    QApplication app(argc, argv);

#if defined(Q_OS_LINUX) && TEST_SIGWATCH
    UnixSignalWatcher sigwatch;
    sigwatch.watchForSignal(SIGINT);
#endif

    app.setOrganizationName(QIPERF_ORG);
    app.setOrganizationDomain(QIPERF_DOMAIN);
    app.setApplicationName(QIPERFC_NAME);
    QStringList styles = QStyleFactory::keys();
    qInfo() << "Available styles:" << styles.join(",");
    app.setStyle(QStyleFactory::create("Fusion"));
    // not work
    // QFile styleFile(":/qiperf.qss"); // If embedded in resources
    // if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
    //     QString styleSheet = styleFile.readAll();
    //     qApp->setStyleSheet(styleSheet); // Apply to the whole application
    // }
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
