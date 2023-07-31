#include "myinfo.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QHostInfo>
#include <QNetworkInterface>

#include <QDebug>


MyInfo::MyInfo(QString mgr_ifname, QObject *parent)
    : QObject{parent}
{
    m_ifname = mgr_ifname;
}

QString MyInfo::collectInfo()
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
   "update":0
 }
*/

    QJsonObject mainObject;
    mainObject.insert("Type", getEndpointType());
    mainObject.insert("HostName", QHostInfo::localHostName());
    mainObject.insert("OS", QSysInfo::prettyProductName());
    mainObject.insert("OSVer", QSysInfo::kernelVersion());
    mainObject.insert("Manager", m_ifname);
    mainObject.insert("update", 0);

    QJsonObject netObject=collectNetInfo();

    mainObject.insert("Net", netObject);
    QJsonDocument jsonDoc;
    jsonDoc.setObject(mainObject);
    //conver to QString
//    QString strJson(jsonDoc.toJson(QJsonDocument::Indented));
    QString strJson(jsonDoc.toJson(QJsonDocument::Compact));
//    qDebug().noquote() << "JSON:" << strJson << Qt::endl;
    return strJson;
}

QJsonObject MyInfo::collectNetInfo()
{
    QJsonObject netObjects;
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
            netObjects.insert(interface.name(), ifObject);
        }
    }
    return netObjects;
}

QList<QHostAddress> MyInfo::getIPfromIfname(QString ifname)
{
    QList<QHostAddress> addrs;
    QHostAddress ipAddress=QHostAddress("0.0.0.0");
    QHostAddress bAddress=QHostAddress("255.255.255.255");

    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    foreach (QNetworkInterface interface, interfaces) {
        if (interface.name() == ifname) {
            QList<QNetworkAddressEntry> addresses = interface.addressEntries();
            foreach (QNetworkAddressEntry address, addresses) {
                if (address.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    ipAddress = address.ip();
                    addrs.append(ipAddress);
                    bAddress = address.broadcast();
                    addrs.append(bAddress);
                    break;
                }
            }
            break;
        }
    }
//    return ipAddress;
    return addrs;
}

int MyInfo::getEndpointType()
{
//    EndPoint::Type rc = EndPoint::Unknown;
    int rc = static_cast<int>(EndPointType::Unknown);
    QStringList myOptions;
    myOptions << "windows" << "android" << "macos" << "osx" << "ios" << "debian" << "unknown" ;

    switch(myOptions.indexOf(QSysInfo::productType())){
    case 0: //windows
        rc = static_cast<int>(EndPointType::Windows);
        break;
    case 1: //android
        rc = static_cast<int>(EndPointType::Android);
        break;

    case 2: //macos
    case 3: //osx
        rc = static_cast<int>(EndPointType::MacOS);
        break;
    case 4: //ios
        rc = static_cast<int>(EndPointType::iOS);
        break;
    case 5: //debian/FreeBSD
        rc = static_cast<int>(EndPointType::FreeBSD);
        break;
    case 6:
        rc = static_cast<int>(EndPointType::Unknown);
        break;
    default:// Linux
        rc = static_cast<int>(EndPointType::Linux);
        break;
    }
    return rc;

//    QString tpy =QSysInfo::productType();
//    if (QString::compare(tpy, "android", Qt::CaseInsensitive)==0){
//    }else if (QString::compare(tpy, "macos", Qt::CaseInsensitive)==0){
//    }
}
