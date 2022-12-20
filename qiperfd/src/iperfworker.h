#ifndef IPERFWORKER_H
#define IPERFWORKER_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

class IperfWorker : public QObject
{
    Q_OBJECT
public:
    explicit IperfWorker(int idx, int version, QString cmd, QString arg="-s", uint port=5201, QObject *parent = nullptr);
    ~IperfWorker();
    void setStop();
signals:
    void started(int idx);
    void finished(int idx, int exitCode, int exitStatus);
    void log(int idx, QString msg);
    void onStdout(int idx, QString text);
    void onStderr(int idx, QString text);

public slots:
    void work();

private slots:
    void onStarted();
    void readyReadStdOut();
    void readyReadStdErr();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    int m_idx;
    int m_version; // iperf version 2 or 3
    QObject *m_parent;
    bool m_stop;
    QString m_iperfexe; // iperf exec name
    uint m_port;  //iperf port
    QString m_cmd; //iperf exec full path
    QStringList m_arguments;  //iperf args
    QProcess *m_iperf; // iperf procress
};

#endif // IPERFWORKER_H
