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
    int add(QString refrow, int version,QString m_cmd,QString args, uint port,
            QString bndaddr="0.0.0.0", QString target="",
            QString parallel="0", QString protocal="TCP",
            bool bidir=false, bool reverse=false, int interval=1,
            int delaytime=0);
    int add(QString refrow, QVariantMap jsondata);
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

public slots:
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

signals:
    void setMgrIfname(QString ifname);
    void iperfStarted(QString bindkey); // iperf thrad started
    void setStop();

protected:
//    void closeEvent(QCloseEvent *event);
private slots:
    void onWSactMessage(QString msg); //procress websocket action message
    void onNewClient(QHostAddress addr); //
private:
    QString tmppath;
    QString tmpfilepath;
    IperfWrapper *m_iperfwrapper;
    QSettings *cfg;
    UdpSrv *m_udpsrv;
    MyInfo *m_myinfo;
#if (TEST_WS==1)
    WSServer *m_wsserver;
#endif
    PipeServer *m_pserver;
    //TODO: iperf1
    QString m_iperfexe2; //iperf2
    QString m_iperfexe21; //iperf2.1
    QString m_iperfexe3; //iperf3
    QMap<int, IperfWorker*> m_iperfworkers;
    QMap<int, QThread*> m_threads; // iperfworker's thread
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
};

#endif // QIPERFD_H
