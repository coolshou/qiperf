#ifndef IPERFWORKER_H
#define IPERFWORKER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>

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
    void setRefRow(QString refrow);
    void toLogFile(QString msg);

signals:
    void started(int idx);
    void finished(int idx, int exitCode, int exitStatus);
    void log(int idx, QString msg);
    void onStdout(int idx, QString text);
    void onStderr(int idx, QString text);


public slots:
    void work();
    bool isRunning();

private slots:
    void onStarted();
    void readyReadStdOut();
    void readyReadStdErr();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString m_refrow;
    int m_idx;
    int m_version; // iperf version 2 or 3
    QObject *m_parent;
    bool m_stop;  //user stop;
    bool m_running; // is running
    bool m_servermode=false;
    QString m_iperfexe; // iperf exec name
    QString m_iperflogpath;
    QFile *m_logfile;
    QTextStream *m_logtextstream;

    uint m_port;  //iperf port
    QString m_bindaddr; // iperf bind address
    QString m_target; //target address
    QString m_cmd; //iperf exec full path
    QStringList m_arguments;  //iperf args
    QProcess *m_iperf; // iperf procress
};

#endif // IPERFWORKER_H
