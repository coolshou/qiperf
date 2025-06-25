#ifndef QIPERFD_H
#define QIPERFD_H

#include <QObject>
#include <QList>
#include <QThread>
#include <QSettings>
#include <QList>
#include <QString>
#include <QDateTime>
#include "iperfwrapper.h"
#include "fileclient.h"

#include "../src/filewatcher.h"
#include "../src/icmpping.h"
#include "serial/serialtask.h"
#include "ssh/sshtask.h"

//#include <QCloseEvent> # require gui

#if defined(Q_OS_LINUX)
//#include <QSocketNotifier>
#endif
#include "comm.h"
#include "pipeserver.h"
#include "iperfworker.h"
//#include "myservice.h"
#include "udpsrv.h"
#include "myinfo.h"
#if (TEST_WS==1)
#include "wsserver.h"
#endif

#if defined(Q_OS_WIN32)
#include <comdef.h> // For _bstr_t, _variant_t, _com_ptr_t
// Important: Import the Task Scheduler type library
// This generates C++ classes and smart pointers for the COM interfaces
#include <taskschd.h>
#pragma comment(lib, "taskschd.lib")

//#import <taskschd.dll> // This line makes your compiler generate taskschd.tlh and taskschd.tli
// Use a namespace alias for clarity
//namespace TS = TaskScheduler;

#endif

#include "../lib/ntp/ntpserver.h"
#include "../lib/ntp/ntpsync.h"

class QIperfd : public QObject
{
    Q_OBJECT
public:
//    explicit QIperfd(QObject *parent = nullptr);
    explicit QIperfd(PipeServer *pserver, QObject *parent = nullptr);
    ~QIperfd() override;

    void onLog(QString text);
    void loadcfg(QString apppath);
    void savecfg();
    //QList<QString> listInterfaces();
    QStringList listInterfaces();
    QString getInterfaceAddr(QString ifname);
    QString getManagerInterface();
    QString getIfNameByHumanReadableName(QString name);
    qint64 add(QString refrow, int version,QString m_cmd,QString args, uint port,
            QString bndaddr="0.0.0.0", QString target="",
            QString parallel="0", QString protocal="TCP",
            bool bidir=false, bool reverse=false, int interval=1,
            int delaytime=0);
    qint64 add(QString refrow, QVariantMap jsondata);
    void del(int idx);
    int addIperfServer(QString refrow, int version, uint port, QString bindHost="");
    int addIperfClient(QString refrow, int version, uint port, QString Host, QString iperfargs);
    void start(int idx); // start idx of iperf
    void startAll(); // start all of iperfs
    void stop(int idx);  // stop idx of iperfs
    void stopAll();  // stop all iperfs
    void clear(); //clear all iperf setting
    bool isRunning(int idx); //check if iperf is running
    void restartQIperfd();
    void infoQIperfdStopped();

public slots:
    void informMessage(QString data, bool bShowAtLocal=false);

    void setManagerInterface(QString ifname);
    void onPipeMessage(int idx, const QString msg);
    void readStdOut(int idx, QString text);
    void onErrored(int m_idx, int refrow, QString text, QString ipport);
    void onIperfLog(int idx, QString text);
    void onStarted(int m_idx, bool smode, QString ipport);
    void onFinished(int idx, int exitCode, int exitStatus, QString ipport, QString filename);
    void onThroughput(int idx, QString sInterval, QString data); // idx, refrow, throughput data
    void onQuit();
    void onNewLine(QString line);
    void doRestartQIperfd();
    void onSerialTaskFinished(QString serialPortName);
    void onSerialTaskStarted(QString idx, quint16 port);
    void onSerialTaskError(QString idx, QString errormsg);
    void onSSHTaskFinished(QString target);
    void onSSHTaskStarted(QString idx, quint16 port);
    void onSSHTaskError(QString idx, QString errormsg);
    void onTimeSynced(QString target, bool synced);
signals:
    void setMgrIfname(QString ifname);
    void iperfStarted(QString bindkey); // iperf thrad started
    void setStop();

protected:
//    void closeEvent(QCloseEvent *event);
private slots:
    void onWSactMessage(QString msg, QHostAddress fromAddr, quint16 fromPort); //procress websocket action message
    void onNewClient(QHostAddress addr); //

private:
    int checkFirewallStatus();
    void setNtpServer(QString mode);
    void startNtpServer();
    void initIperf(QString apppath);
    void getIperfVer(QString cmd, double ver);
    // QString getIperf2ver();
    // QString getIperf21ver();
    // void getIperfVersion();
#if defined(Q_OS_WINDOWS)
    // Helper function to create a task using COM API
    bool createScheduledTask(const QString &taskName, const QString &taskCommand, const QString &taskArgs, const QDateTime &runTime);
    bool runScheduledTask(const QString &taskName);
    bool deleteScheduledTask(const QString &taskName);

    // Helper for HRESULT error codes
    QString comErrorToString(HRESULT hr);

#endif
    QString apppath;
    QString tmppath;
    QString qiperfdlog;
    QString tmpfilepath;
    QString m_nssm;
    IperfWrapper *m_iperfwrapper;
    QSettings *cfg;
    UdpSrv *m_udpsrv;
    MyInfo *m_myinfo;
#if (TEST_WS==1)
    WSServer *m_wsserver;
#endif
    PipeServer *m_pserver;
    //TODO: iperf1
    QString m_iperfexe2; // iperf2 use version
    QString m_iperfexe20; //iperf2.0
    QString m_iperfexe20ver; //iperf2 version
    QString m_iperfexe21; //iperf2.1
    QString m_iperfexe21ver; //iperf2.1 version
    QString m_iperfexe22; //iperf2.2
    QString m_iperfexe22ver; //iperf2.2 version
    QString m_iperfexe3; //iperf3
    QString m_iperfexe3ver; //iperf3 version
    QMap<qint64, IperfWorker*> m_iperfworkers;
    QMap<qint64, QThread*> m_threads; // iperfworker's thread
//    QList<IperfWorker*> m_iperfworkers;
//    QList<QThread*> m_threads;
    QString mgr_ifname; //manager interface name
    int mgr_port; //manager port number
    QDateTime m_starttime;  //Test Start time
    QString s_starttime;   // Test Start time in string;
    QMap<int, int> m_runstatus; //record thread idx run status, 0: stop , 1: running
    bool bReportTPData; // report throughput data
    QMap<QString, QMap<QString, QString>> m_directions; // starttime, bindkey, dir_tag: direction tag for each test
    FileClient *m_fileclient;
    FileWatcher *m_filewatcher;
    IcmpPing *m_icmpping;
    bool bNtpserver; // enable NTP server
    NtpServer *m_ntpserver;
    NtpSync *m_ntpsync;
    void initiperf2(QString tmp, QString tmp_path, QString arch);
    void initiperf21(QString tmp, QString tmp_path, QString arch);
    void initiperf22(QString tmp, QString tmp_path, QString arch);
    void initiperf3(QString tmp, QString tmp_path, QString arch);
    // serial
    QMap<QString, SerialTask*> m_serialtasks;
    QMap<QString, SSHTask*> m_sshtasks;
};

#endif // QIPERFD_H
