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
    /*/convert val: Kbits/sec, Mbits/sec, Gbits/sec, Tbits/sec,
     *              KBytes/sec, MBytes/sec, GBytes/sec, TBytes/sec,
     * to Kbps, Mbps, Gbps, Tbps, KB/s, MB/s, GB/s, TB/s
    */
    QString rs="Mbps";
    if (QString::compare(val, "Kbits/sec", Qt::CaseInsensitive) == 0) {
        rs="Kbps";
    }else if (QString::compare(val, "Mbits/sec", Qt::CaseInsensitive) == 0) {
        rs="Mbps";
    }else if (QString::compare(val, "Gbits/sec", Qt::CaseInsensitive) == 0) {
        rs="Gbps";
    }else if (QString::compare(val, "Tbits/sec", Qt::CaseInsensitive) == 0) {
        rs="Tbps";
    }else if (QString::compare(val, "KBytes/sec", Qt::CaseInsensitive) == 0) {
        rs="KB/s";
    }else if (QString::compare(val, "MBytes/sec", Qt::CaseInsensitive) == 0) {
        rs="MB/s";
    }else if (QString::compare(val, "GBytes/sec", Qt::CaseInsensitive) == 0) {
        rs="GB/s";
    }else if (QString::compare(val, "TBytes/sec", Qt::CaseInsensitive) == 0) {
        rs="TB/s";
    }else {
        rs="Mbps";
        qInfo() << "NOT SUPPORT Unit: " << val << " , return:" << rs;
    }
    return rs;
}
