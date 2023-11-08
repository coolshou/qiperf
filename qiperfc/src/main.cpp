#include "qiperfc.h"

#include <QApplication>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include "../src/comm.h"

#define TEST_SIGWATCH 1
#if defined(Q_OS_LINUX) && TEST_SIGWATCH
#include "../qt-unix-signals/sigwatch.h"
#endif

static QTextStream output_ts;
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
//        fprintf(stderr, "DEBUG: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        output_ts << QString("DEBUG: %1 (%2:%3)").arg(msg).arg(file).arg(context.line) << Qt::endl;
        break;
    case QtInfoMsg:
        //fprintf(stderr, "INFO: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
//        fprintf(stderr, "INFO: %s \n", localMsg.constData());
        output_ts << QString("INFO: %1 ").arg(msg) << Qt::endl;
        break;
    case QtWarningMsg:
//        fprintf(stderr, "WARN: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        output_ts << QString("WARN: %1 (%2:%3)").arg(msg).arg(file).arg(context.line) << Qt::endl;
        break;
    case QtCriticalMsg:
//        fprintf(stderr, "CRITICAL: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        output_ts << QString("CRITICAL: %1 (%2:%3)").arg(msg).arg(file).arg(context.line) << Qt::endl;
        break;
    case QtFatalMsg:
//        fprintf(stderr, "FATAL: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        output_ts << QString("FATAL: %1 (%2:%3)").arg(msg).arg(file).arg(context.line) << Qt::endl;
        break;
    }
}

int main(int argc, char *argv[])
{
    //TODO: log file
    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    QString logfilePath = tmp + "/qiperf/";
    QDir dir(logfilePath);
    if (!dir.exists())
        dir.mkpath(".");
    QString logfile = logfilePath + "qiperfc.log";
    QFile outFile(logfile);
    if (! outFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        qDebug() << "open file " << logfile << " Fail" << Qt::endl;
    }
    output_ts.setDevice(&outFile);
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
    return app.exec();
}
