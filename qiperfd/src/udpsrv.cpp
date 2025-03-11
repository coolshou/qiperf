#include "udpsrv.h"
#include <QDateTime>

#include <QDebug>

UdpSrv::UdpSrv(quint16 port, QString mgr_ifname, MyInfo *myinfo, QObject *parent)
    : QObject{parent}
{
    m_port = port;
    m_ifname = mgr_ifname;
    m_myinfo = myinfo;

    socket = new QUdpSocket(this);

    update_addr();

    //We need to bind the UDP socket to an address and a port
//    socket->bind(m_addr);
    //sendMsg = "Test";

//    connect(socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
    infomer=new QTimer(this);
    connect(infomer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    infomer->start(5*1000); // 5 sec
}

void UdpSrv::onLog(QString text)
{
    qInfo() << "UdpSrv:" << text;
}

void UdpSrv::setIfname(QString mgr_ifname)
{
    m_ifname = mgr_ifname;
    update_addr();
    qInfo() << "UdpSrv::setIfname:" << mgr_ifname;
}

void UdpSrv::readyRead()
{
    QByteArray Buffer;
    Buffer.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram(Buffer.data(),Buffer.size(),&sender,&senderPort);

    qDebug() << "UdpSrv::readyRead:" << Buffer;
}

void UdpSrv::onTimeout()
{
    qint64 length=0;
    if (m_sendMsg.length()>0){
        //qInfo() << "onTimeout: send to : " << m_baddr << " port:" << m_port << " MSG:" << m_sendMsg;
        QString tmp=m_sendMsg;
        // length = socket->writeDatagram(tmp.toLatin1(),tmp.length(),
        //  writeDatagram Max 1479 bytes?
        length = socket->writeDatagram(tmp.toUtf8(),tmp.length(),
                                       QHostAddress::Broadcast, m_port);
                                       //m_baddr, m_port);

        if (length<0){
            qInfo() << "ERROR writeDatagram ("<< QString::number(socket->error()) <<"):" << socket->errorString();
            return;
        }else {
            qDebug() << "length:" << tmp.length() << " send length:" << length;
        }
        // don't clear, let it keeps sending
//        m_sendMsg ="";
    } else {
        qInfo() <<"("<< QString::number(m_sendMsg.length()) <<")" "wait new m_sendMsg";
    }
}

void UdpSrv::update_addr()
{
    QList<QHostAddress> addrs;
    addrs = m_myinfo->getIPfromIfname(m_ifname);
    if (addrs.length()>0){
//        socket->unbind();
        m_addr = addrs[0]; // ip address
        m_baddr = addrs[1]; // broadcast address
        if (!(socket->ConnectedState == QAbstractSocket::UnconnectedState)) {
            onLog("update_addr: m_addr:" + m_addr.toString());
            socket->bind(m_addr, QUdpSocket::ShareAddress); // now interface
        }
    }else{
        qDebug() << "Did not find any address for interface: " << m_ifname;
    }
}

void UdpSrv::setSendMsg(QString msg)
{   //NOTICE: this did not send immedially!!
    if (msg.length()>0){
        m_sendMsg = msg;
    }
}
