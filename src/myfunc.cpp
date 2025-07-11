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
    if ((val=="Kbits")||
        (QString::compare(val, "Kbits/sec", Qt::CaseInsensitive) == 0)) {
        rs="Kbps";
    }else if ((val=="Mbits")||
               (QString::compare(val, "Mbits/sec", Qt::CaseInsensitive) == 0)) {
        rs="Mbps";
    }else if ((val=="Gbits")||
               (QString::compare(val, "Gbits/sec", Qt::CaseInsensitive) == 0)) {
        rs="Gbps";
    }else if ((val=="Tbits")||
               (QString::compare(val, "Tbits/sec", Qt::CaseInsensitive) == 0)) {
        rs="Tbps";
    }else if ((val=="KBytes")||
               (QString::compare(val, "KBytes/sec", Qt::CaseInsensitive) == 0)) {
        rs="KB/s";
    }else if ((val=="MBytes")||
               (QString::compare(val, "MBytes/sec", Qt::CaseInsensitive) == 0)) {
        rs="MB/s";
    }else if ((val=="GBytes")||
               (QString::compare(val, "GBytes/sec", Qt::CaseInsensitive) == 0)) {
        rs="GB/s";
    }else if ((val=="TBytes")||
               (QString::compare(val, "TBytes/sec", Qt::CaseInsensitive) == 0)) {
        rs="TB/s";
    }else {
        rs="Mbps";
        qInfo() << "NOT SUPPORT Unit: " << val << " , return:" << rs;
    }
    return rs;
}

QString MyFunc::secToHumanReadable(long long seconds)
{
    // Handle negative numbers
    if (seconds < 0) {
        return "Invalid input: seconds cannot be negative";
    }

    // Handle zero
    if (seconds == 0) {
        return "0 seconds";
    }
    TimeComponents tcompon = secondsToComponents((int)seconds);
    QString result="";
    if (tcompon.days>0){
        result = QString("%1 days").arg(QString::number(tcompon.days));
    }
    if (tcompon.hours>0){
        if (!result.isEmpty()){
            result = result + ", ";
        }
        result = result + QString("%1 hours").arg(QString::number(tcompon.hours));
    }
    if (tcompon.minutes>0){
        if (!result.isEmpty()){
            result = result + ", ";
        }
        result = result + QString("%1 mins").arg(QString::number(tcompon.minutes));
    }
    if (tcompon.seconds>0){
        if (!result.isEmpty()){
            result = result + ", ";
        }
        result = result + QString("%1 sec").arg(QString::number(tcompon.seconds));
    }
    // QString result = QString("%1 day, %2 hour, %3 min, %4 sec")
    //                      .arg(QString::number(tcompon.days),
    //                           QString::number(tcompon.hours),
    //                           QString::number(tcompon.minutes),
    //                           QString::number(tcompon.seconds));
    return result;
}

TimeComponents MyFunc::secondsToComponents(int totalSeconds)
{
    TimeComponents result;
    result.days = totalSeconds / 86400;
    result.hours = (totalSeconds % 86400) / 3600;
    result.minutes = (totalSeconds % 3600) / 60;
    result.seconds = totalSeconds % 60;
    return result;
}
