#include "udpsrv.h"
#include <QDateTime>

#include <QDebug>

UdpSrv::UdpSrv(quint16 port, QString mgr_ifname, MyInfo *myinfo, QObject *parent)
    : QObject{parent}
{
    m_port = port;
    m_ifname = mgr_ifname;
    m_myinfo = myinfo;

    update_addr();

    socket = new QUdpSocket(this);
    //We need to bind the UDP socket to an address and a port
    socket->bind(m_addr);
    //sendMsg = "Test";

//    connect(socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
    infomer=new QTimer(this);
    connect(infomer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    infomer->start(5*1000); // 5 sec
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
    if (m_sendMsg.length()>0){
//        qDebug() << "("<< QString::number(m_sendMsg.length()) <<")"<<"onTimeout=" <<QString(m_sendMsg) << "." << Qt::endl;
        QDateTime t=QDateTime::currentDateTime();
        QString tmp= t.toString("yyyy.dd.MM.hh:mm:ss.zzz")+":"+ m_sendMsg;
        length = socket->writeDatagram(tmp.toLatin1(),tmp.length(),
                                       m_baddr, m_port);
//                                       QHostAddress::Broadcast, m_port);
        if (length<0){
            qDebug() << "ERROR writeDatagram ("<< QString(socket->error()) <<"):" << socket->errorString() << Qt::endl;
            return;
        }
        // don't clear, let it keeps sending
//        m_sendMsg ="";
//        qDebug() <<"("<< QString::number(m_sendMsg.length()) <<")" "clear m_sendMsg:" << m_sendMsg << "." << Qt::endl;
    } else {
        qDebug() <<"("<< QString::number(m_sendMsg.length()) <<")" "wait new m_sendMsg" << Qt::endl;
    }
}

void UdpSrv::update_addr()
{
    QList<QHostAddress> addrs;
    addrs = m_myinfo->getIPfromIfname(m_ifname);
    if (addrs.length()>0){
//        socket->unbind();
        m_addr = addrs[0];
        m_baddr = addrs[1];
//        qDebug() << "m_addr:" << m_addr.toString() << Qt::endl;
//        qDebug() << "m_baddr:" << m_baddr.toString() << Qt::endl;
        socket->bind(m_addr); // now interface
    }
}

void UdpSrv::setSendMsg(QString msg)
{
    m_sendMsg = msg;
}
