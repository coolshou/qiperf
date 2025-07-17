#ifndef TPWORKER_H
#define TPWORKER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <QList>
#include <QDateTime>
#include <QMutex>

#include "tp.h"
#include "tpstatus.h"
#include "wsclient.h"
#include "tpflowmanager.h"
// #include "tpstatuschecker.h"

class TpWorker : public QObject
{
    Q_OBJECT
public:
    explicit TpWorker(QString logpath, QList<TP *> &tps, bool ignoreWrongInterval,
                      int wstimeout=10,
                      int extrawait = 5, int waitserverready = 10,
                      QObject *parent = nullptr);
    ~TpWorker() override;

    void work();
    void onStop();
public slots:
    void onSetStop();
    void onStatusReady(QString refrow, const QString& skey,
                       TPStatus::Status status);
    void onWaitTimeout(QString refrow, const QString& skey);
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
    // Signals to communicate the overall result of the HighLevelWorker's task back to the main thread
    void workFinished(const QString& result); // e.g., "Success", "Timeout", "Error"

private slots:
    void onIperfStarted(QString smode, QString ipport);
    void onIperfReStarted(QString smode, QString ipport);
    void onIperfStoped(QString refrow, QString err_no, QString err, QString ipport);
    void onServerDisconnected(QString targetip);
    void onClientDisconnected(QString targetip);
    void onDisconnected(QString targetip);
    void onIperfTPdata(QString refrow, QString sInterval, QString datas);
    void onIperfExtendWait(QString refrow, qint64 iwait, int exitCode);
    bool isStatusServersRunning(QStringList &ds, bool bAll=false);
    bool isStatusClientsRunning(QStringList &ds, bool bAll=false);
    int getStatusServers();
    int getStatusClients();
    void onDebuginfo(QString msg);
    // for m_statusChecker
    void provideCurrentStatus(const QString& key, QString smode="S");
    void handleStatusAchieved(const QString& key, TPStatus::Status status);
    void handleTimeoutOccurred(const QString& key);
    void handleMonitoringFinished();
private:
    void initStart();
    void resetError();
    void debug(QString msg, int debuglv=3);
    int m_debuglv;
    QString m_logpath;
    QList<TP *> m_tps;
    bool m_ignoreWrongInterval;
    int iWSTimeout; // default wait websocket timeout 10
    int iExtraWait = 5;
    int m_WaitServerReady;
    qint64 m_extendwaittime;
    QString m_datapath;
    QDateTime m_TestStartTime;
    bool bUserStop;
    int bErrorStop;
    QString m_ErrorMSG;
    QMap<QString, WSClient *> m_ws; // websocket list for manager qiperfd's iperf
    // QMap<QString, WSClient *> m_wss; // websocket client list for manager iperf server
    // QMap<QString, WSClient *> m_wsc; // websocket client list for manager iperf client
    QMap<QString, TPStatus::Status> m_status_server; // store server status, 0: init, 1: running, 2: error?
    QMap<QString, TPStatus::Status> m_status_client; // store client status, 0: init, 1: running, 2: error?
    QMutex m_mutex;
    TpFlowManager *m_flowManager;
    bool waitAllServerReady();
    bool waitServerReady(QString skey, TPStatus::Status expectstatus);
    bool waitAllClientReady();
    bool waitClientReady(QString skey, TPStatus::Status expectstatus);
    // TPStatusChecker *m_statusChecker;
};

#endif // TPWORKER_H
