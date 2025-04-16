#include "comdevicetcp.h"

#include <QHostAddress>
#include <QDebug>

ComDeviceTcp::ComDeviceTcp(const QString& localIp, const QString& localPort,
                           Mode mode, QObject *parent)
    : ComDevice{parent}
    , _localIp(localIp)
    , _localPort(localPort)
    , _mode(mode)
{}

ComDeviceTcp::~ComDeviceTcp()
{
    close();
}

void ComDeviceTcp::init()
{
    _tcpServer = new QTcpServer(this);

    connect(_tcpServer, &QTcpServer::acceptError, this, &ComDeviceTcp::slotAcceptError);
    connect(_tcpServer, &QTcpServer::newConnection, this, &ComDeviceTcp::slotNewConnection);

    QHostAddress hostAddress = _localIp.compare("any", Qt::CaseInsensitive) == 0 ? QHostAddress::Any : QHostAddress(_localIp);
    if (hostAddress.isNull()) {
        // L_ERROR("TCP-Server listen address invalid");
        qDebug() << "TCP-Server listen address invalid";
        emit finished();
        return;
    }

    bool ok = false;
    quint16 localPort = _localPort.toUShort(&ok);
    if (!ok) {
        // L_ERROR("TCP-Server local port invalid");
        qDebug() << "TCP-Server local port invalid";
        emit finished();
        return;
    }

    if (!_tcpServer->listen(hostAddress, localPort)) {
        // L_ERROR("TCP-Server listen failed");
        qDebug() << "TCP-Server listen failed";
        emit finished();
        return;
    }
    // L_NOTE(QString("TCP-Server listening: %1 %2").arg(hostAddress.toString()).arg(_localPort));
    qInfo() << QString("TCP-Server listening: %1 %2").arg(hostAddress.toString()).arg(_localPort);
}

void ComDeviceTcp::slotDataSend(const QByteArray &data)
{
    if (!_tcpServer) {
        return;
    }

    foreach (QTcpSocket* tcpSocket, _tcpSocketList) {
        qint64 number = tcpSocket->write(data);
        if (number == -1) {
            // L_ERROR("TCP-Socket write failed");
            qDebug() << "TCP-Socket write failed";
            emit finished();
        }
        else if (number != data.size()) {
            // L_WARN("TCP-Socket write partial data"); // TODO
            qWarning() << "TCP-Socket write partial data";
            emit finished();
        }
    }
}

void ComDeviceTcp::close()
{
    if (_tcpServer) {
        foreach (QTcpSocket* tcpSocket, _tcpSocketList) {
            tcpSocket->disconnectFromHost();
            // L_NOTE("TCP-Socket closed");
            qInfo() << "TCP-Socket closed";
        }
        _tcpServer->close();
        // L_NOTE("TCP-Server closed");
        qInfo() << "TCP-Server closed";
    }
}

void ComDeviceTcp::slotAcceptError(QAbstractSocket::SocketError socketError)
{
    // L_ERROR(QString("TCP-Server accept error: %1").arg(socketError));
    qDebug() << QString("TCP-Server accept error: %1").arg(socketError);
    emit finished();
}

void ComDeviceTcp::slotNewConnection()
{
    if (!_tcpServer) {
        return;
    }

    QTcpSocket* tcpSocket = _tcpServer->nextPendingConnection();
    if (!tcpSocket) {
        return;
    }

    if (_mode == Mode::BINARY && !_tcpSocketList.isEmpty()) {
        // limit to one connection
        tcpSocket->disconnectFromHost();
        return;
    }

    connect(tcpSocket, &QAbstractSocket::disconnected, this, &ComDeviceTcp::slotDisconnected);
    connect(tcpSocket, &QIODevice::readyRead, this, &ComDeviceTcp::slotReadyRead);

    _tcpSocketList << tcpSocket;

    // L_NOTE(QString("TCP-Socket connected: %1 %2").arg(tcpSocket->peerAddress().toString()).arg(tcpSocket->peerPort()));
    qInfo() << QString("TCP-Socket connected: %1 %2").arg(tcpSocket->peerAddress().toString()).arg(tcpSocket->peerPort());
}

void ComDeviceTcp::slotDisconnected()
{
    QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
    if (!tcpSocket) {
        return;
    }
    if (_mode == Mode::TEXT) {
        QString key = peerString(tcpSocket); // map key
        if (_dataRecv.remove(key) > 0) {
            // L_DEBUG(QString("key remove: %1").arg(key));
            qDebug() << QString("key remove: %1").arg(key);
        }
    }
    _tcpSocketList.removeAll(tcpSocket);
    tcpSocket->deleteLater();
    // L_NOTE(QString("TCP-Socket closed: %1 %2").arg(tcpSocket->peerAddress().toString()).arg(tcpSocket->peerPort()));
    qInfo() << QString("TCP-Socket closed: %1 %2").arg(tcpSocket->peerAddress().toString()).arg(tcpSocket->peerPort());
}

void ComDeviceTcp::slotReadyRead()
{
    QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());
    if (!tcpSocket) {
        return;
    }
    QByteArray data = tcpSocket->readAll();
    if (_mode == Mode::BINARY) {
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
            // L_DEBUG(QString("dataRecvListTrimmed.size: %1").arg(dataRecvListTrimmed.size()));
            QByteArray data = dataRecv.toUtf8();
            data.append("\n");
            emit signalDataRecv(data);
        }
    }
}

QString ComDeviceTcp::peerString(const QTcpSocket *tcpSocket)
{
    if (!tcpSocket) {
        return "-";
    }
    return QString("%1:%2").arg(tcpSocket->peerAddress().toString()).arg(tcpSocket->peerPort());
}
