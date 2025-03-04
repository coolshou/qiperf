#include "myfunc.h"

// MyFunc::MyFunc(QObject *parent): QObject(parent)
// {

// }

bool MyFunc::isValidIpAddress(const QString &ip, int &protocal) {
    //check if ip is valid IPv4/IPv6 address, update protocal to correct value
    QHostAddress address;
    if (address.setAddress(ip)) {
        // Check if it's a valid IPv4 or IPv6 address
        if (address.protocol() == QAbstractSocket::IPv4Protocol){
            protocal = QAbstractSocket::IPv4Protocol;
        }
        if (address.protocol() == QAbstractSocket::IPv6Protocol){
            protocal = QAbstractSocket::IPv6Protocol;
        }
        return (address.protocol() == QAbstractSocket::IPv4Protocol ||
                address.protocol() == QAbstractSocket::IPv6Protocol);
    }
    protocal = -1;
    return false;  // Not a valid IP address
}

QString MyFunc::formatUnit(QString val)
{
    /*/convert val: Kbits, Mbits, Gbits, Tbits, KBytes,MBytes, GBytes, TBytes,
     * to Kbps, Mbps, Gbps, Tbps, KB/s, MB/s, GB/s, TB/s
    */
    QString rs="Mbps";
    if (QString::compare(val, "Kbits", Qt::CaseInsensitive) == 0) {
        rs="Kbps";
    }else if (QString::compare(val, "Mbits", Qt::CaseInsensitive) == 0) {
        rs="Mbps";
    }else if (QString::compare(val, "Gbits", Qt::CaseInsensitive) == 0) {
        rs="Gbps";
    }else if (QString::compare(val, "Tbits", Qt::CaseInsensitive) == 0) {
        rs="Tbps";
    }else if (QString::compare(val, "KBytes", Qt::CaseInsensitive) == 0) {
        rs="KB/s";
    }else if (QString::compare(val, "MBytes", Qt::CaseInsensitive) == 0) {
        rs="MB/s";
    }else if (QString::compare(val, "GBytes", Qt::CaseInsensitive) == 0) {
        rs="GB/s";
    }else if (QString::compare(val, "TBytes", Qt::CaseInsensitive) == 0) {
        rs="TB/s";
    }else {
        rs="Mbps";
        qInfo() << "NOT SUPPORT Unit: " << val << " , return:" << rs;
    }
    return rs;
}
