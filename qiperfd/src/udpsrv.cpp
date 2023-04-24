#include "udpsrv.h"
#include <QDateTime>
#include <QSysInfo>
#include <QNetworkInterface>
#include <QHostInfo>

#include <QDebug>

UdpSrv::UdpSrv(quint16 port, QString mgr_ifname, QObject *parent)
    : QObject{parent}
{
    m_port = port;
    m_ifname = mgr_ifname;

    socket = new QUdpSocket(this);
    //We need to bind the UDP socket to an address and a port
//    socket->bind(QHostAddress::Broadcast, port);
    //sendMsg = "Test";

//    connect(socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
    infomer=new QTimer(this);
    connect(infomer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    infomer->start(5*1000);
}

void UdpSrv::readyRead()
{
    QByteArray Buffer;
    Buffer.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram(Buffer.data(),Buffer.size(),&sender,&senderPort);
}

void UdpSrv::onTimeout()
{
    qint64 length=0;
    if (sendMsg != ""){
        QDateTime t=QDateTime::currentDateTime();
        QString tmp= t.toString("yyyy.dd.MM.hh:mm:ss.zzz")+":"+ sendMsg;
        qDebug() << "going to send:" << tmp << Qt::endl;
        length = socket->writeDatagram(tmp.toLatin1(),tmp.length(),
                                        QHostAddress::Broadcast, m_port);
        if (length){

            return;
        }

    }
}
