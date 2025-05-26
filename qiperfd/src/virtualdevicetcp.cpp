#include "virtualdevicetcp.h"

#include <QHostAddress>
#include <QDebug>

VirtualDeviceTcp::VirtualDeviceTcp(QString idx, const QString& localIp, const QString& localPort,
                           Mode mode,
                           QObject *parent)
    : VirtualDevice{parent}
    , m_idx(idx)
    , _localIp(localIp)
    , _localPort(localPort)
    , _mode(mode)
{}

VirtualDeviceTcp::~VirtualDeviceTcp()
{
    if (_tcpServer){
        _tcpServer->close();
    }
}

bool VirtualDeviceTcp::isRunning()
{
    if (_tcpServer){
        return _tcpServer->isListening();
    }else{
        return false;
    }
}

void VirtualDeviceTcp::init()
{
    _tcpServer = new QTcpServer(this);

    connect(_tcpServer, &QTcpServer::acceptError, this, &VirtualDeviceTcp::slotAcceptError);
    connect(_tcpServer, &QTcpServer::newConnection, this, &VirtualDeviceTcp::slotNewConnection);

    QHostAddress hostAddress = _localIp.compare("any", Qt::CaseInsensitive) == 0 ? QHostAddress::Any : QHostAddress(_localIp);
    if (hostAddress.isNull()) {
        qDebug() << "TCP-Server listen address invalid";
        emit finished();
        return;
    }

    bool ok = false;
    quint16 localPort = _localPort.toUShort(&ok);
    if (!ok) {
        qDebug() << "TCP-Server local port invalid";
        emit finished();
        return;
    }

    if (!_tcpServer->listen(hostAddress, localPort)) {
        qDebug() << "TCP-Server listen failed";
        emit finished();
        return;
    }
    qInfo() << QString("TCP-Server listening: %1 %2").arg(hostAddress.toString(), _localPort);
    emit started(m_idx, localPort);
}

void VirtualDeviceTcp::slotDataSend(const QByteArray &data)
{
    if (!_tcpServer) {
        qDebug() << "VirtualDeviceTcp::slotDataSend No _tcpServer";
        return;
    }
    foreach (QTcpSocket* tcpSocket, _tcpSocketList) {
        if (!tcpSocket->isWritable()){
            qDebug() << "tcpSocket is NOT Writable";
        }
        qint64 number = tcpSocket->write(data);
        if (number == -1) {
            qDebug() << "TCP-Socket write failed";
            emit finished();
        }
        else if (number != data.size()) {
            qWarning() << "TCP-Socket write partial data";
            emit finished();
        }
    }
}

void VirtualDeviceTcp::close()
{
    if (_tcpServer) {
        foreach (QTcpSocket* tcpSocket, _tcpSocketList) {
            tcpSocket->disconnectFromHost();
            qInfo() << "TCP-Socket closed";
        }
        _tcpServer->close();
        qInfo() << "TCP-Server closed";
    }
}

void VirtualDeviceTcp::slotAcceptError(QAbstractSocket::SocketError socketError)
{
    qDebug() << QString("TCP-Server accept error: %1").arg(socketError);
    emit finished();
}

void VirtualDeviceTcp::slotNewConnection()
{
    if (!_tcpServer) {
        return;
    }

    QTcpSocket* tcpSocket = _tcpServer->nextPendingConnection();
    if (!tcpSocket) {
        return;
    }

    if (_mode == Mode::BINARY && !_tcpSocketList.isEmpty()) {
        qDebug() << "VirtualDeviceTcp // limit to one connection: " << _tcpSocketList;
        tcpSocket->disconnectFromHost();
        return;
    }

    connect(tcpSocket, &QAbstractSocket::disconnected, this, &VirtualDeviceTcp::slotDisconnected);
    connect(tcpSocket, &QIODevice::readyRead, this, &VirtualDeviceTcp::slotReadyRead);

    _tcpSocketList << tcpSocket;

    // qInfo() << QString("TCP-Socket connected");
    // %1 %2").arg(tcpSocket->peerAddress().toString(), tcpSocket->peerPort());
}

void VirtualDeviceTcp::slotDisconnected()
{
    QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
    if (!tcpSocket) {
        qDebug() << "VirtualDeviceTcp::slotDisconnected sender not tcpSocket:" << sender();
        return;
    }
    if (_mode == Mode::TEXT) {
        QString key = peerString(tcpSocket); // map key
        if (_dataRecv.remove(key) > 0) {
            qDebug() << QString("key remove: %1").arg(key);
        }
    }
    _tcpSocketList.removeAll(tcpSocket);
    QString ip = tcpSocket->peerAddress().toString();
    QString port = QString::number(tcpSocket->peerPort());
    tcpSocket->deleteLater();
    qInfo() << QString("TCP-Socket closed: %1 %2").arg(ip, port);
}

void VirtualDeviceTcp::slotReadyRead()
{
    QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
    if (!tcpSocket) {
        return;
    }
    QByteArray data = tcpSocket->readAll();
    if (_mode == Mode::BINARY) {
        // qDebug() << "VirtualDeviceTcp::slotReadyRead" << data;
        emit signalDataRecv(data);
        return;
    }
    // handle LF in multi-tcp-connection mode (Mode::TEXT)
    QString key = peerString(tcpSocket); // map key
    _dataRecv[key] += QString::fromUtf8(data);
    if (_dataRecv[key].contains("\n")) {
        const QStringList dataRecvList = _dataRecv[key].split("\n");
        QStringList dataRecvListTrimmed;
        for (const QString& dataRecv : dataRecvList) {
            dataRecvListTrimmed.push_back(dataRecv.trimmed());
        }
        while (dataRecvListTrimmed.size() >= 2) {
            QString dataRecv = dataRecvListTrimmed.takeFirst();
            _dataRecv[key] = dataRecvListTrimmed.join("\n");
            QByteArray data = dataRecv.toUtf8();
            data.append("\n");
            emit signalDataRecv(data);
        }
    }
}

QString VirtualDeviceTcp::peerString(const QTcpSocket *tcpSocket)
{
    if (!tcpSocket) {
        qDebug() << "VirtualDeviceTcp::peerString no tcpSocket, return -";
        return "-";
    }
    return QString("%1:%2").arg(tcpSocket->peerAddress().toString(), tcpSocket->peerPort());
}
