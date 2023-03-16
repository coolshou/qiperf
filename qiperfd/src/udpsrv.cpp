#include "udpsrv.h"
#include <QDateTime>
#include <QSysInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkInterface>
#include <QHostInfo>

#include <QDebug>

UdpSrv::UdpSrv(int port, QString mgr_ifname, QObject *parent)
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

QString UdpSrv::collectInfo()
{
    //collect Info for use , in json format string
/*
 { "HostName": "Test"
   "OS":"", ""OSVer",
   "Manager": "ifname1",
   "Net": {
       "ifname1": {
            "HW":"",
            "Addrs":[["ip", "Mask", "BCast"],["ip2", "Mask2", "BCast2"]],
       "ifname2": {
            "HW":"",
            "Addrs":[["ip", "Mask", "BCast"],["ip2", "Mask2", "BCast2"]]
   },

 }
*/

    QJsonObject mainObject;
    mainObject.insert("HostName", QHostInfo::localHostName());
    mainObject.insert("OS", QSysInfo::prettyProductName());
    mainObject.insert("OSVer", QSysInfo::kernelVersion());
    mainObject.insert("Manager", m_ifname);

    QJsonObject netObject;

    //獲取所有網路介面的列表
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface interface,list) //遍歷每一個網路介面
    {
        if ((interface.type() == QNetworkInterface::Ethernet) ||
                (interface.type() == QNetworkInterface::Wifi)) {
            QJsonObject ifObject;
            ifObject.insert("HW", interface.hardwareAddress()); //硬體地址

            QJsonArray addrsObject;
            //獲取IP地址條目列表，每個條目中包含一個IP地址，一個子網掩碼和一個廣播地址
            QList<QNetworkAddressEntry> entryList= interface.addressEntries();
            foreach(QNetworkAddressEntry entry,entryList)//遍歷每個IP地址條目
            {
                QJsonArray addrObject;
                addrObject.push_back(entry.ip().toString());//IP地址
                addrObject.push_back(entry.netmask().toString());//子網掩碼
                addrObject.push_back(entry.broadcast().toString());//廣播地址
                addrsObject.push_back(addrObject);
            }
            ifObject.insert("address", addrsObject);
            netObject.insert(interface.name(), ifObject);
        }
    }

    mainObject.insert("Net", netObject);
    QJsonDocument jsonDoc;
    jsonDoc.setObject(mainObject);
    //conver to QString
    QString strJson(jsonDoc.toJson(QJsonDocument::Indented));
    qDebug().noquote() << "JSON:" << strJson << Qt::endl;
    return strJson;

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
    int length=0;
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
