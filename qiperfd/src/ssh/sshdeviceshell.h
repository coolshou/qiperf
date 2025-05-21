#ifndef SSHDEVICESHELL_H
#define SSHDEVICESHELL_H

#include <QObject>
#include "../virtualdevice.h"
#include <qssh/sshconnection.h>
#include <qssh/sshremoteprocess.h>

using namespace QSsh;
namespace QSsh {
class SshConnection;
class SshConnectionParameters;
class SshRemoteProcess;
}

class SSHDeviceShell : public VirtualDevice
{
    Q_OBJECT
public:
    explicit SSHDeviceShell(const QSsh::SshConnectionParameters &parameters,
                            QObject *parent = nullptr);
    ~SSHDeviceShell();
    void run();
    bool isRunning();
public slots:
    virtual void init();
    virtual void slotDataSend(const QByteArray& data);
    void close();
    // void slotReadyRead();

private:
    void handleConnected();
    void handleDisconnected();
    void handleConnectionError();
    void handleRemoteStdout();
    void handleRemoteStderr();
    void handleShellMessage(const QString &message);
    void handleChannelClosed(int exitStatus);
    void handleShellStarted();
    void handleStdin();

    QSsh::SshConnection *m_connection;
    QSharedPointer<QSsh::SshRemoteProcess> m_shell;
    bool _connected;

};

#endif // SSHDEVICESHELL_H
