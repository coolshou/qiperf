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
    explicit IperfWorker(int ver, QString cmd, QString arg="-s", uint port=5201, QObject *parent = nullptr);
    ~IperfWorker();
    void setStop();
signals:
    void started();
    void finished(int exitCode, int exitStatus);
    void log(QString msg);
    void onStdout(QString text);
    void onStderr(QString text);

public slots:
    void work();

private slots:
    void onStarted();
    void readyReadStdOut();
    void readyReadStdErr();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
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
