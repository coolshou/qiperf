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
#include <QDebug>

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

#if defined(Q_OS_WIN32)
// --- BEGIN: Helper RAII Class (Highly Recommended) ---
// This class ensures CoInitializeEx/CoUninitialize are paired correctly per thread.
// Place this in its own header (e.g., "cominitializer.h")
class ComInitializer {
public:
    explicit ComInitializer(DWORD dwCoInit = COINIT_MULTITHREADED) : m_initialized(false) {
        HRESULT hres = CoInitializeEx(nullptr, dwCoInit);
        if (SUCCEEDED(hres)) {
            if (hres == S_OK) { // S_OK means COM was initialized by this call
                m_initialized = true;
            }
            // S_FALSE means COM was already initialized on this thread with the same model.
            // We don't need to uninitialize it later if this call returned S_FALSE.
        } else {
            // Only log if it's a true failure, not RPC_E_CHANGED_MODE
            if (hres != RPC_E_CHANGED_MODE) {
                qCritical() << "Failed to initialize COM for this thread. HRESULT:"
                            << QString("0x%1").arg(static_cast<unsigned int>(hres), 8, 16, QChar('0').toUpper());
            } else {
                qWarning() << "COM already initialized on this thread with a different apartment model (RPC_E_CHANGED_MODE). HRESULT:"
                           << QString("0x%1").arg(static_cast<unsigned int>(hres), 8, 16, QChar('0').toUpper());
                // Depending on your needs, you might want to consider this a fatal error
                // if the desired MTA is strictly required and cannot be met.
            }
        }
    }

    ~ComInitializer() {
        if (m_initialized) {
            CoUninitialize();
            qDebug() << "COM uninitialized for this thread.";
        }
    }

    bool isInitialized() const { return m_initialized; }

private:
    bool m_initialized;
    ComInitializer(const ComInitializer&) = delete;
    ComInitializer& operator=(const ComInitializer&) = delete;
};
// --- END: Helper RAII Class ---
#endif

int main(int argc, char *argv[])
{
    int rc;
    rc = isNotRoot();
    if (rc == 0){
#ifdef Q_OS_WIN32
        // Use the RAII helper for COM initialization
        // It handles CoInitializeEx return values (S_OK, S_FALSE, FAILED) correctly
        ComInitializer com_init(COINIT_MULTITHREADED);

        if (!com_init.isInitialized()) {
            // If com_init.isInitialized() is false AND the HRESULT from CoInitializeEx
            // was a true FAILED (not S_FALSE or RPC_E_CHANGED_MODE),
            // then critical error.
            // The ComInitializer constructor already logged the error for us.
            QMessageBox::critical(nullptr, "COM Initialization Error",
                                  "Failed to initialize COM for the application.");
            return 1; // Exit if COM cannot be set up
        }

        // Initialize COM security for the process/thread after CoInitializeEx.
        // Do this only ONCE per application lifecycle (or once per thread if different contexts).
        // The MyInfo class should NOT call CoInitializeSecurity again.
        HRESULT hr_sec = CoInitializeSecurity(
            NULL,                          // Security descriptor
            -1,                            // Use default authentication service
            NULL,                          // Authentication services list
            NULL,                          // Reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,     // Default authentication level for proxies
            RPC_C_IMP_LEVEL_IMPERSONATE,   // Default impersonation level for proxies
            NULL,                          // Authentication info
            EOAC_NONE,                     // Additional capabilities
            NULL                           // Reserved
            );

        if (FAILED(hr_sec) && hr_sec != S_FALSE) { // S_FALSE means it was already initialized
            qWarning() << "Failed to initialize COM security:" << getHResultErrorString(hr_sec);
            // Depending on the severity, you might want to return here.
            // WMI calls might fail if security isn't set up correctly.
        } else if (hr_sec == S_FALSE) {
            qDebug() << "COM security already initialized (S_FALSE).";
        } else {
            qDebug() << "COM security initialized successfully.";
        }
#endif // Q_OS_WIN32

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
    // #if defined(Q_OS_WIN32)
    //     // Initialize COM for the main thread here, once at startup
    //     // This is typically called once per thread that uses COM.
    //     HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); // Or COINIT_MULTITHREADED
    //     if (FAILED(hr)) {
    //         QMessageBox::critical(nullptr, "COM Initialization Error",
    //                               QString("Failed to initialize COM library: 0x%1").arg(hr, 8, 16, QChar('0').toUpper()));
    //         return 1;
    //     }
    // #endif
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
// #if defined(Q_OS_WIN32)
//     // Uninitialize COM when the application exits
//     CoUninitialize();
// #endif
    return rc;
}
