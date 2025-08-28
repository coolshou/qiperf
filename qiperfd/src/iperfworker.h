#ifndef IPERFWORKER_H
#define IPERFWORKER_H
/*
 * this class wii run in QThread
*/
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDateTime>

#include "iperfwrapper.h"

class IperfWorker : public QObject
{
    Q_OBJECT
public:
    explicit IperfWorker(qint64 idx, int version, QString cmd, QString arg="-s",
                         uint port=5201, QString bindaddr="0.0.0.0", QString target="",
                         bool bidir=false, bool reverse=false, int interval=1,
                         qint64 duration=30,
                         int delaystart=0, bool ignoreWrongInterval=false,
                         bool restartonerror=false, QJsonObject restartrule = QJsonObject(),
                         QString tmplogpath = "/tmp",
                         QObject *parent = nullptr);
    ~IperfWorker() override;
    QString getBindKey(); // return  bind_addr:port
    void setBidirTag(QString bidir);
    void setRefRow(QString refrow);
    void setExtra(QString parallel, QString protocal, uint port);
    void toLogFile(QString msg);
    bool getServerMode();
    int getRefRow();
    qint64 getPID();
    void debug(QString msg, int debuglv=3);
    QString getThreadID();

signals:
    void workerFinished(qint64 idx, bool servermode); // Signal to notify manager that this worker is done
    void started(int refrow, bool smode, QString ipport); //start: refrow, S/C, IPPort
    void restarted(int refrow, bool smode, QString ipport); //restart: refrow, S/C, IPPort
    void finished(int refrow, int exitCode, int exitStatus, QString ipport, QString filename, bool servermode); // refrow, exitcode, exitStatus, , IPPort

    void log(int idx, QString msg); // refrow
    // void onStdout(int idx, QString text); // refrow
    void onStderr(int idx, int refrow, QString text, QString ipport); // idx, refrow, msg, ipport
    void iperfTPdata(int refrow, QString sInterval,  QString data); // refrow, sInterval, throughput data
    void iperfExtendWait(int refrow, qint64 iwait, int exitCode, int restarttimes); // refrow, extra wait time(sec), error code, restart count
    void stopSelfDestructor();
    void debuginfo(QString msg);


public slots:
    void work();
    void setStop();
    void onWorkerRestart();
    bool isRunning();
    void onSelfDestructor();
    void onSetDebugLv(int lv);
    void onSetStartTime(QString stime);
    void onSetReStartTime(int idx, QDateTime restime);
    void onSetReStart(bool isServer);
    void setIperfLogPath(QString filepath); //full path of iperf log filename
private slots:
    void onStarted();
    void onRestart();
    void onNoStdout();
    void readyReadStdOut();
    void readyReadStdErr();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void parserStdOut(QString msg);
    void onThroughputData(int refrow, QString sInterval,  QString data);
    void onDebuginfo(QString msg);


private:
    QString m_threadid; // real thread id
    int m_selfdestructionTime;
    QTimer *m_selfdestruction;
    QTimer *m_restarter; // timer to do restart;
    int mStdoutDetectTime;
    QTimer *m_stdoutdetect; // detect when no stdout for some time, consider the iperf -s is stoped
    int m_refrow;// refrow
    qint64 m_idx;
    int m_version; // iperf version 2 or 3
    QString m_cmd; //iperf exec full path
    uint m_port;  //iperf port
    QString m_bindaddr; // iperf bind address
    QString m_target; //target address
    bool m_bidir=false;
    bool m_reverse=false;
    int m_interval=1;  // report interval
    int m_duration;
    int m_delaystart;
    bool m_ignoreWrongInterval;
    bool m_restartonerror;
    bool m_restartonErrorStop;
    bool m_restartonNormalStop;
    QJsonObject m_restartrule;
    QJsonArray m_restartrules; // array of rule
    QString m_tmplogpath;
    QObject *m_parent;
    IperfWrapper *m_iperfwrapper;
    bool m_stop;  //user stop;
    bool m_running; // is running
    bool m_servermode=false;
    QString m_parallel="0";
    QString m_protocal="TCP";

    QString m_iperfexe; // iperf exec name
    QString m_iperflogpath;
    QString m_bidirtag;
    QFile *m_logfile;
    QTextStream *m_logtextstream;

    QStringList m_arguments;  //iperf args
    int m_omit=0;
    QProcess *m_iperf; // iperf procress
    QMap<QString, QJsonArray> m_tpdatas;
    int m_debuglv;

    bool m_restarttimes;  //count how many times it do restart
    QDateTime m_starttime; // test start time
    QMap<int, QDateTime> m_restarttimemap; //record each restart time stemp
};

#endif // IPERFWORKER_H
