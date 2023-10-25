#include "qiperfc.h"

#include <QApplication>
#include "../src/comm.h"

#define TEST_SIGWATCH 1
#if defined(Q_OS_LINUX) && TEST_SIGWATCH
#include "../qt-unix-signals/sigwatch.h"
#endif


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
        fprintf(stderr, "INFO: %s \n", localMsg.constData());
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

int main(int argc, char *argv[])
{
    //TODO: log file
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
