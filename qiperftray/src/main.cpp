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
#include <QDateTime>

#include <qlogging.h>


#include <stdio.h>


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
}
int main(int argc, char *argv[])
{
    //log file
    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    QString logfilePath = tmp + QDir::separator() + QIPERF_NAME + QDir::separator();
    QDir dir(logfilePath);
    if (!dir.exists())
        dir.mkpath(".");
    QString logfile = logfilePath + QIPERFTRAY_NAME + ".log";
    // TODO: check log file exist, backup it
    QFile outFile(logfile);
    if (! outFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        qDebug() << "open file " << logfile << " Fail";
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
    w.setLogFile(logfile);
    //w.setWindowFlags( Qt::WindowTitleHint |  Qt::WindowMinimizeButtonHint | Qt::WindowSystemMenuHint);


    QObject::connect(mytray, &MyTray::sigShow, &w, &QIperfTray::show);
    QObject::connect(mytray, &MyTray::sigQuit, &w, &QApplication::quit);
    QObject::connect(mytray, &MyTray::sigIconActivated, &w, &QIperfTray::onTrayIconActivated);
//    w.show();
    return app.exec();
}
