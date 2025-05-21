#ifndef SSHTASK_H
#define SSHTASK_H

#include <QObject>

#include "sshdeviceshell.h"
#include "../virtualdevicetcp.h"
#include "sshdeviceshell.h"

using namespace QSsh;

class SSHTask : public QObject
{
    Q_OBJECT
public:
    explicit SSHTask(QString midx, const QString& sshTarget, const QString& sshPort,
                     const QString& localIp, const QString& localPort,
                     VirtualDeviceTcp::Mode mode,
                     QString username, QString password,
                     QString privateKeyFile="", int timeout=30,
                     QObject *parent = nullptr);
    ~SSHTask();
    quint16 getLocalPort();
    QString getIdx();
    bool isRunning();
    void setConfig(QString localPort, QString sshTarget, QString sshPort,
                   QString username, QString password, QString privateKeyFile,
                   int timeout);
    QString getLastError();
public slots:
    void init();
    void close();
    void onStarted(QString idx, quint16 port);

signals:
    void finished(QString sshTarget);
    void started(QString idx, quint16 port);// notice run on which port
private slots:
    void slotFinished();
private:
    Q_DISABLE_COPY(SSHTask)
private:
    QString m_idx;
    QString _sshTarget;
    QString _sshPort;
    QString _localIp;
    QString _localPort;
    const VirtualDeviceTcp::Mode _mode;
    QString _username;
    QString _password;
    QString _sshPrivateKeyFile;
    int _timeout;
    QString _lasterror;

    SSHDeviceShell *_sshDeviceShell;
    VirtualDeviceTcp *_DeviceTcp;

};

#endif // SSHTASK_H
