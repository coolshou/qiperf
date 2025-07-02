#ifndef TPWORKER_H
#define TPWORKER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <QList>
#include <QDateTime>

#include "tp.h"
#include "wsclient.h"

class TpWorker : public QObject
{
    Q_OBJECT
public:
    explicit TpWorker(QString logpath, QList<TP *> &tps, int wstimeout=10,
                      int extrawait = 5, int waitserverready = 10,
                      QObject *parent = nullptr);
    ~TpWorker() override;

    void work();
    void onStop();
public slots:
    void onSetStop();

signals:
    void testStarted();
    void testStoped(int err); // signal when test stoped, 0: no error
    void updateDatapath(QString datapath);
    void updateStarttime(QDateTime starttime);
    void updateRunStatus(bool bStart);
    void updateComment(QString refrow, QString errmsg);
    void updateStatus(QString msg);
    void iperfTPdata(QString refrow, QString sInterval, QString data); // refrow, throughput data
    void errorStop(int err, QString msg); // signal when test error
    void updateInterval(int interval);
    void setEndTime(double value);
    void debuginfo(QString msg);

private slots:
    void onIperfStarted(QString smode, QString ipport);
    void onIperfStoped(QString refrow, QString err_no, QString err, QString ipport);
    void onServerDisconnected(QString targetip);
    void onClientDisconnected(QString targetip);
    void onDisconnected(QString targetip);
    void onIperfTPdata(QString refrow, QString sInterval, QString datas);
    int getStatusServers();
    int getStatusClients();
    void onDebuginfo(QString msg);

private:
    void initStart();
    void resetError();
    void debug(QString msg, int debuglv=3);
    int m_debuglv;
    QString m_logpath;
    QList<TP *> m_tps;
    int iWSTimeout; // default wait websocket timeout 10
    int iExtraWait = 5;
    int m_WaitServerReady;

    QString m_datapath;
    QDateTime m_TestStartTime;
    bool bUserStop;
    int bErrorStop;
    QString m_ErrorMSG;
    QMap<QString, WSClient *> m_ws; // websocket list for manager qiperfd's iperf
    // QMap<QString, WSClient *> m_wss; // websocket client list for manager iperf server
    // QMap<QString, WSClient *> m_wsc; // websocket client list for manager iperf client
    QMap<QString, int> m_status_server; // store server status, 0: init, 1: running, 2: error?
    QMap<QString, int> m_status_client; // store client status, 0: init, 1: running, 2: error?

    bool waitServerReady();
    bool waitClientReady();
};

#endif // TPWORKER_H
