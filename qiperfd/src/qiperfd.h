#ifndef QIPERFD_H
#define QIPERFD_H

#include <QObject>
#include <QList>
#include <QThread>
#include <QSettings>
#include <QList>
#include <QString>

#include "pipeserver.h"
#include "iperfworker.h"
//#include "myservice.h"
#include "udpsrv.h"
#include "myinfo.h"

class QIperfd : public QObject
{
    Q_OBJECT
public:
    explicit QIperfd(QObject *parent = nullptr);
//    ~QIperfd();
    void onLog(QString text);
    void loadcfg(QString apppath);
    void savecfg();
    QList<QString> listInterfaces();
    QString getInterfaceAddr(QString ifname);
    QString getManagerInterface();
    void setMgr_ifname(QString ifname);
    void add(int version,QString m_cmd,QString args, uint port);
    void start();
    void stop();
    //TODO: start all iperfs
    //TODO: stop all iperfs

public slots:
    void setManagerInterface(QString interface);
    void onNewMessage(int idx, const QString msg);
    void readStdOut(int idx, QString text);
    void readStdErr(int idx, QString text);
    void onIperfLog(int idx, QString text);
    void onStarted(int idx);
    void onFinished(int idx, int exitCode, int exitStatus);
    void onQuit();
signals:
private:
    QSettings cfg;
    UdpSrv *m_udpsrv;
    MyInfo *m_myinfo;

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

};

#endif // QIPERFD_H
