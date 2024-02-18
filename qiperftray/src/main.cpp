#include "qiperftray.h"

#include <QApplication>
#include <QObject>
#include "../src/comm.h"
#include "src/mytray.h"
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
    //log file
    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    QString logfilePath = tmp + "/qiperf/";
    QDir dir(logfilePath);
    if (!dir.exists())
        dir.mkpath(".");
    QString logfile = logfilePath + "qiperftray.log";
    // TODO: check log file exist, backup it
    QFile outFile(logfile);
    if (! outFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        qDebug() << "open file " << logfile << " Fail" << Qt::endl;
    } else {
        output_ts.setDevice(&outFile);
    }
    qInstallMessageHandler(myMessageOutput);

    QApplication app(argc, argv);
    app.setOrganizationName(QIPERF_ORG);
    app.setOrganizationDomain(QIPERF_DOMAIN);
    app.setApplicationName(QIPERFTRAY_NAME);

    MyTray * mytray = new MyTray();
    QIperfTray w(mytray);
    //w.setWindowFlags( Qt::WindowTitleHint |  Qt::WindowMinimizeButtonHint | Qt::WindowSystemMenuHint);


    QObject::connect(mytray, &MyTray::sigShow, &w, &QIperfTray::show);
    QObject::connect(mytray, &MyTray::sigQuit, &w, &QApplication::quit);
    QObject::connect(mytray, &MyTray::sigIconActivated, &w, &QIperfTray::onTrayIconActivated);
//    w.show();
    return app.exec();
}
