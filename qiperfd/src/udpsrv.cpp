#include "udpsrv.h"
#include <QDateTime>

#include <QDebug>

UdpSrv::UdpSrv(quint16 port, QString mgr_ifname, MyInfo *myinfo, QObject *parent)
    : QObject{parent}
{
    m_debuglv = 3;
    m_port = port;
    m_ifname = mgr_ifname;
    m_myinfo = myinfo;
    socket = new QUdpSocket(this);
    //We need to bind the UDP socket to an address and a port
    update_addr();
    infomer=new QTimer(this);
    connect(infomer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    infomer->start(5*1000); // 5 sec
}

void UdpSrv::debug(QString text, int lv)
{
    if (lv <m_debuglv){
        qInfo() << "[UdpSrv]" << text;
    }
}

void UdpSrv::setIfname(QString mgr_ifname)
{
    m_ifname = mgr_ifname;
    update_addr();
    debug("UdpSrv::setIfname:" + mgr_ifname);
}

void UdpSrv::onSetDebugLv(int lv)
{
    m_debuglv = lv;
}

void UdpSrv::readyRead()
{
    QByteArray Buffer;
    Buffer.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram(Buffer.data(),Buffer.size(),&sender,&senderPort);

    debug(QString("UdpSrv::readyRead:%1").arg(Buffer));
}

void UdpSrv::onTimeout()
{
    qint64 length=0;
    if (m_sendMsg.length()>0){
        QString tmp=m_sendMsg;
        length = socket->writeDatagram(tmp.toUtf8(),
                                       QHostAddress::Broadcast, m_port);
                                       //m_baddr, m_port);
        if (length<0){
            debug("ERROR writeDatagram ("+ QString::number(socket->error()) + "):"
                  + socket->errorString()
                  + " Broadcast on port " + QString::number(m_port)
                  + " msg:" + tmp);
            return;
        }
        // }else if (length != tmp.toUtf8().length()){
        //     qDebug() << "expect length:" << tmp.toUtf8().size() << " ,actual send length:" << length;
        // }
        // don't clear m_sendMsg, let it keeps sending
        // m_sendMsg ="";
    } else {
        debug("("+ QString::number(m_sendMsg.length()) +")" "wait new m_sendMsg");
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
            debug("update_addr: m_addr:" + m_addr.toString());
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
