#ifndef IPERFWORKER_H
#define IPERFWORKER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QJsonArray>

#include "iperfwrapper.h"

class IperfWorker : public QObject
{
    Q_OBJECT
public:
    explicit IperfWorker(int idx, int version, QString cmd, QString arg="-s",
                         uint port=5201, QString bindaddr="0.0.0.0", QString target="",
                         QObject *parent = nullptr);
    ~IperfWorker() override;
    void setStop();
    QString getBindKey(); // return  bind_addr:port
    void setIperfLogPath(QString filepath); //full path of iperf log filename
    void setBidirTag(QString bidir);
    void setRefRow(QString refrow);
    void setExtra(QString parallel, QString protocal,bool bidir);
    void toLogFile(QString msg);

signals:
    void started(int idx); // refrow
    void finished(int idx, int exitCode, int exitStatus); // refrow
    void log(int idx, QString msg); // refrow
    void onStdout(int idx, QString text); // refrow
    void onStderr(int idx, QString text); // refrow
    void onThroughput(int idx, QString sInterval,  QString data); // refrow, sInterval, throughput data


public slots:
    void work();
    bool isRunning();


private slots:
    void onStarted();
    void readyReadStdOut();
    void readyReadStdErr();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void parserStdOut(QString msg);
    void onThroughputData(int idx, QString sInterval,  QString data);
//    void parserIperf3(QString msg);

private:
    QString m_refrow;
    int m_idx;  // refrow
    int m_version; // iperf version 2 or 3
    QObject *m_parent;
    IperfWrapper *m_iperfwrapper;
    bool m_stop;  //user stop;
    bool m_running; // is running
    bool m_servermode=false;
    QString m_parallel="0";
    QString m_protocal="TCP";
    bool m_bidir=false;
    bool m_reverse=false; //TODO
    QString m_iperfexe; // iperf exec name
    QString m_iperflogpath;
    QString m_bidirtag;
    QFile *m_logfile;
    QTextStream *m_logtextstream;
    uint m_port;  //iperf port
    QString m_bindaddr; // iperf bind address
    QString m_target; //target address
    QString m_cmd; //iperf exec full path
    QStringList m_arguments;  //iperf args
    QProcess *m_iperf; // iperf procress
    QMap<QString, QJsonArray> m_tpdatas;

};

#endif // IPERFWORKER_H
