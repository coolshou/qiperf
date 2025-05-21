#include "sshtask.h"

#include <qssh/sshconnection.h>
#include <QTimer>

SSHTask::SSHTask(QString midx, const QString& sshTarget, const QString& sshPort,
                 const QString& localIp, const QString& localPort,
                 VirtualDeviceTcp::Mode mode,
                 QString username, QString password,
                 QString privateKeyFile, int timeout, QObject *parent)
    : QObject{parent}, m_idx(midx), _sshTarget(sshTarget), _sshPort(sshPort),
    _localIp(localIp), _localPort(localPort), _mode(mode),
    _username(username), _password(password), _sshPrivateKeyFile(privateKeyFile),
    _timeout(timeout)
{
    _lasterror="";
}

SSHTask::~SSHTask()
{

}

quint16 SSHTask::getLocalPort()
{
    return _localPort.toUInt();
}

QString SSHTask::getIdx()
{
    return m_idx;
}

bool SSHTask::isRunning()
{
    if (_sshDeviceShell && _DeviceTcp){
        if (_sshDeviceShell->isRunning() && _DeviceTcp->isRunning()){
            return true;
        }else{
            if (!_sshDeviceShell->isRunning()){
                _lasterror = "_sshDeviceShell not running";
                qDebug() << _lasterror;
            }
            if (!_DeviceTcp->isRunning()){
                _lasterror = _lasterror + " _DeviceTcp not running";
                qDebug() << _lasterror;
            }
            return false;
        }
    }else {
        _lasterror = "_sshDeviceShell or _DeviceTcp not exist";
        return false;
    }
}

void SSHTask::setConfig(QString localPort, QString sshTarget, QString sshPort, QString username, QString password, QString privateKeyFile, int timeout)
{
    _localPort = localPort;
    _sshTarget = sshTarget;
    _sshPort = sshPort;
    _username = username;
    _password = password;
    _sshPrivateKeyFile = privateKeyFile;
    _timeout = timeout;

}

QString SSHTask::getLastError()
{
    return _lasterror;
}

void SSHTask::init()
{
    try{
        SshConnectionParameters para = SshConnectionParameters();
        para.setHost(_sshTarget);
        para.setPort(_sshPort.toInt());
        if(_sshPrivateKeyFile.isEmpty()){
            para.authenticationType = SshConnectionParameters::AuthenticationTypeTryAllPasswordBasedMethods;
            para.setUserName(_username);
            para.setPassword(_password);
        }else{
            para.authenticationType = SshConnectionParameters::AuthenticationTypePublicKey;
            para.privateKeyFile = _sshPrivateKeyFile;
        }
        para.timeout = _timeout;
        // if (!para.host().isEmpty())
        {
            try{
                _sshDeviceShell = new SSHDeviceShell(para, this);
            } catch (const std::exception &e) {
                qDebug() << "SSHDeviceShell error: " << e.what();
            }
            try{
                qDebug() << QString("new VirtualDeviceTcp: %1,%2,%3").arg(m_idx, _localIp, _localPort);
                _DeviceTcp = new VirtualDeviceTcp(m_idx, _localIp, _localPort, _mode, this);
            } catch (const std::exception &e) {
                qDebug() << "VirtualDeviceTcp error: " << e.what();
            }

            connect(_sshDeviceShell, &VirtualDevice::finished, this, &SSHTask::slotFinished);
            connect(_DeviceTcp, &VirtualDevice::finished, this, &SSHTask::slotFinished);
            connect(_DeviceTcp, &VirtualDeviceTcp::started, this, &SSHTask::onStarted);

            connect(_sshDeviceShell, &VirtualDevice::signalDataRecv, _DeviceTcp, &VirtualDevice::slotDataSend);
            connect(_DeviceTcp, &VirtualDevice::signalDataRecv, _sshDeviceShell, &VirtualDevice::slotDataSend);

            QTimer::singleShot(0, _sshDeviceShell, SLOT(init()));
            QTimer::singleShot(0, _DeviceTcp, SLOT(init()));
        }
    } catch (const std::exception &e) {
        qDebug() << "SshConnectionParameters " << e.what();
    }
}

void SSHTask::close()
{
    if(_sshDeviceShell){
        _sshDeviceShell->close();
    }
    if(_DeviceTcp){
        _DeviceTcp->close();
    }
}

void SSHTask::onStarted(QString idx, quint16 port)
{
    emit started(idx, port);
}

void SSHTask::slotFinished()
{
    emit finished(_sshTarget);
}
