#include "udpreceiver.h"

#include <QDebug>

UdpReceiver::UdpReceiver(quint16 port, QObject *parent)
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
        QHostAddress send_addr;
        quint16 send_port;
        qint64 recSize=m_socket->pendingDatagramSize();
        datagram.resize(static_cast<int>(recSize));
        qint64 rc=0;
        rc=m_socket->readDatagram(datagram.data(), datagram.size(), &send_addr, &send_port);
        if (rc == -1){
            qDebug() << "UdpReceiver::dataReceived error!!" << Qt::endl;
        } else{
            QString s_addr="";
            bool conversionOK = false;
            QHostAddress ip4Address(send_addr.toIPv4Address(&conversionOK));
//            QString ip4String;
            if (conversionOK)
            {
                s_addr = ip4Address.toString();
            }else{
                s_addr = send_addr.toString();
                qDebug() << s_addr << " convert to ipv4 fail" << Qt::endl;
            }
//            if (!send_addr.isNull()) {
////                s_addr = send_addr.toString();
//                s_addr = send_addr.toIPv4Address();
//            }
            QString msg = datagram.data();
            emit notice(s_addr, msg);
        }
    }
}
