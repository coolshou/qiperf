#include "udpreceiver.h"

UdpReceiver::UdpReceiver(int port, QObject *parent)
    : QObject{parent}
{
    m_socket = new QUdpSocket(this);
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
    m_socket->bind(port, QUdpSocket::ShareAddress);
}

void UdpReceiver::dataReceived()
{
    while(m_socket->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(datagram.data(), datagram.size());
        QString msg = datagram.data();
        emit notice(msg);
    }
}
