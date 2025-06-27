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
#include <QJsonArray>
#include <QTimer>

#include "iperfwrapper.h"

class IperfWorker : public QObject
{
    Q_OBJECT
public:
    explicit IperfWorker(qint64 idx, int version, QString cmd, QString arg="-s",
                         uint port=5201, QString bindaddr="0.0.0.0", QString target="",
                         bool bidir=false, bool reverse=false, int interval=1,
                         int delaystart=0,
                         QObject *parent = nullptr);
    ~IperfWorker() override;
    void setStop();
    QString getBindKey(); // return  bind_addr:port
    void setIperfLogPath(QString filepath); //full path of iperf log filename
    void setBidirTag(QString bidir);
    void setRefRow(QString refrow);
    void setExtra(QString parallel, QString protocal, uint port);
    void toLogFile(QString msg);
    bool getServerMode();
    int getRefRow();
    void debug(QString msg, int debuglv=1);

signals:
    void started(int idx, bool smode, QString ipport); // refrow, S/C, IPPort
    void finished(int idx, int exitCode, int exitStatus, QString ipport, QString filename); // refrow, exitcode, exitStatus, , IPPort
    void log(int idx, QString msg); // refrow
    void onStdout(int idx, QString text); // refrow
    void onStderr(int idx, int refrow, QString text, QString ipport); // idx, refrow, msg, ipport
    void onThroughput(int idx, QString sInterval,  QString data); // refrow, sInterval, throughput data
    void stopSelfDestructor();
    void debuginfo(QString msg);

public slots:
    void work();
    bool isRunning();
    void onSelfDestructor();

private slots:
    void onStarted();
    void readyReadStdOut();
    void readyReadStdErr();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void parserStdOut(QString msg);
    void onThroughputData(int idx, QString sInterval,  QString data);
    void onDebuginfo(QString msg);
//    void parserIperf3(QString msg);

private:
    int m_selfdestructionTime;
    QTimer *m_selfdestruction;
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
    int m_delaystart;
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

};

#endif // IPERFWORKER_H
