#ifndef QIPERFD_H
#define QIPERFD_H

#include <QObject>
#include <QList>
#include <QThread>
#include <QSettings>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QCloseEvent>

#if defined(Q_OS_LINUX)
#include <QSocketNotifier>
#endif
#include "comm.h"
#include "pipeserver.h"
#include "iperfworker.h"
//#include "myservice.h"
#include "udpsrv.h"
#include "myinfo.h"
#include "wsserver.h"

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
    QList<QString> listInterfaces();
    QString getInterfaceAddr(QString ifname);
    QString getManagerInterface();
    int add(int version,QString m_cmd,QString args, uint port);
    int addIperfServer(int version, uint port, QString bindHost="");
    int addIperfClient(int version, uint port, QString Host, QString iperfargs);
    void start(int idx); //TODO: start idx of iperf
    void startAll(); // start all of iperfs
    void stop(int idx);  //TODO: stop idx of iperfs
    void stopAll();  //TODO: stop all iperfs
    bool isRunning(int idx); //check if iperf is running

public slots:
    void setManagerInterface(QString interface);
    void onPipeMessage(int idx, const QString msg);
    void readStdOut(int idx, QString text);
    void readStdErr(int idx, QString text);
    void onIperfLog(int idx, QString text);
    void onStarted(int idx);
    void onFinished(int idx, int exitCode, int exitStatus);
    void onQuit();
signals:
    void setMgrIfname(QString interface);
protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void onWSactMessage(QString msg);

private:
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
    QMap<int, QThread*> m_threads;
//    QList<IperfWorker*> m_iperfworkers;
//    QList<QThread*> m_threads;
    QString mgr_ifname; //manager interface name
    int mgr_port; //manager port number
    QDateTime m_starttime;  //Test Start time
    QMap<int, int> m_runstatus; //record thread idx run status, 0: stop , 1: running
};

#endif // QIPERFD_H
