#include "sshdeviceshell.h"

#include <QSocketNotifier>
#include <QDebug>

SSHDeviceShell::SSHDeviceShell(const QSsh::SshConnectionParameters &parameters,
                               QObject *parent)
    : VirtualDevice{parent}//, m_connection(new QSsh::SshConnection(parameters))
{
    _connected = false;
    m_connection = new SshConnection(parameters, this);
    connect(m_connection, &SshConnection::connected, this, &SSHDeviceShell::handleConnected);
    connect(m_connection, &SshConnection::disconnected, this, &SSHDeviceShell::handleDisconnected);
    connect(m_connection, &SshConnection::dataAvailable, this, &SSHDeviceShell::handleShellMessage);
    connect(m_connection, &SshConnection::error, this, &SSHDeviceShell::handleConnectionError);
}

SSHDeviceShell::~SSHDeviceShell()
{
    delete m_connection;
}

void SSHDeviceShell::run()
{
    // if (!m_stdin->open(stdin, QIODevice::ReadOnly | QIODevice::Unbuffered)) {
    //     std::cerr << "Error: Cannot read from standard input." << std::endl;
    //     QCoreApplication::exit(EXIT_FAILURE);
    //     return;
    // }
    init();
}

bool SSHDeviceShell::isRunning()
{
    return _connected;
}

void SSHDeviceShell::init()
{
    m_connection->connectToHost();
}

void SSHDeviceShell::slotDataSend(const QByteArray &data)
{
    //write data to shell
    if (m_shell){
        if (m_shell->isWritable()){
            m_shell->write(data);
        }else{
            qDebug() << "m_shell is not Writable!!";
        }
    }else {
        qDebug() << "No m_shell";
    }
}

void SSHDeviceShell::close()
{
    m_connection->disconnectFromHost();
}

// void SSHDeviceShell::slotReadyRead()
// {
//     qDebug() << "//TODO: slotReadyRead";
//     //read data from shell

// }

void SSHDeviceShell::handleConnectionError()
{
    QString errmsg = QString("SSH connection error: %1").arg(qPrintable(m_connection->errorString()));
    emit signalDataRecv(errmsg.toUtf8());
}

void SSHDeviceShell::handleShellMessage(const QString &message)
{
    qDebug() << "SSHDeviceShell::handleShellMessage: " << qPrintable(message);
}

void SSHDeviceShell::handleConnected()
{
    m_shell = m_connection->createRemoteShell();
    connect(m_shell.data(), &QSsh::SshRemoteProcess::started, this, &SSHDeviceShell::handleShellStarted);
    connect(m_shell.data(), &QSsh::SshRemoteProcess::readyReadStandardOutput,
            this, &SSHDeviceShell::handleRemoteStdout);
    connect(m_shell.data(), &QSsh::SshRemoteProcess::readyReadStandardError,
            this, &SSHDeviceShell::handleRemoteStderr);
    connect(m_shell.data(), &QSsh::SshRemoteProcess::closed, this, &SSHDeviceShell::handleChannelClosed);
    m_shell->start();
}

void SSHDeviceShell::handleDisconnected()
{
    qDebug() << "TODO: SSHDeviceShell::handleDisconnected";
}

void SSHDeviceShell::handleShellStarted()
{
    _connected = true;
    qDebug() << "TODO: handleShellStarted: ";
    // QSocketNotifier * const notifier = new QSocketNotifier(0, QSocketNotifier::Read, this);
    // connect(notifier, &QSocketNotifier::activated, this, &SSHDeviceShell::handleStdin);
}

void SSHDeviceShell::handleRemoteStdout()
{
    QByteArray data = m_shell->readAllStandardOutput().data();
    emit signalDataRecv(data);
}

void SSHDeviceShell::handleRemoteStderr()
{
    QByteArray data = m_shell->readAllStandardError().data();
    qDebug() << "TODO: handleRemoteStderr: " << data;
    emit signalDataRecv(data);
}

void SSHDeviceShell::handleChannelClosed(int exitStatus)
{
    _connected = false;
    qDebug() << "TODO: SSHDeviceShell::handleChannelClosed Shell closed. Exit status was " << exitStatus << ", exit code was "
             << m_shell->exitCode() << ".";
}

void SSHDeviceShell::handleStdin()
{
    //this will keep reading stdin to shell
    qDebug() << "TODO: SSHDeviceShell::handleStdin";
    // m_shell->write(m_stdin->readLine());
}
