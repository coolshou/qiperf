#include "pipeclient.h"

PipeClient::PipeClient(QString remoteServername, QObject *parent)
    : QObject{parent}
{
    m_socket = new QLocalSocket(this);
    m_serverName = remoteServername;

    connect(m_socket, SIGNAL(connected()), this, SLOT(socket_connected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(socket_readReady()));
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError)),
            this, SLOT(socket_error(QLocalSocket::LocalSocketError)));
}

PipeClient::~PipeClient()
{
    m_socket->abort();
    delete m_socket;
    m_socket = NULL;
}

void PipeClient::SetAppHandle(QCoreApplication *app)
{
    m_AppHandle = app;
}

void PipeClient::send_MessageToServer(QString message)
{
    m_socket->abort();
    m_message = message;
    m_socket->connectToServer(m_serverName);
}

void PipeClient::socket_connected()
{   //when socket connected, send m_message to server
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << m_message;
    out.device()->seek(0);
    m_socket->write(block);
    m_socket->flush();

    qDebug() << "socket connetcted";
}

void PipeClient::socket_disconnected()
{
    qDebug() << "socket_disconnected";
}

void PipeClient::socket_readReady()
{
    //qDebug() << "socket_readReady";
    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_5_15);
    if (m_socket->bytesAvailable() < (int)sizeof(quint16)) {
        return;
    }
    QString message;
    in >> message;
    qDebug() << "Client got Msg : " << message;
    emit newMessage(message);
    send_MessageToServer("OK");
}

void PipeClient::socket_error(QLocalSocket::LocalSocketError err)
{
    if (err == QLocalSocket::ConnectionRefusedError){
        emit sigError(QString("Can not connect to %1, check %1 process is running!!").arg(m_serverName));
    } else {
        qDebug() << "socket_error:" << err << Qt::endl;
    }
}
