#include "myfunc.h"

#include <QtMath>
#include <QNetworkInterface>

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

QString MyFunc::closeCodeToString(QWebSocketProtocol::CloseCode code) {
    switch (code) {
    case QWebSocketProtocol::CloseCodeNormal:
        return "Normal Closure";
    case QWebSocketProtocol::CloseCodeGoingAway:
        return "Going Away";
    case QWebSocketProtocol::CloseCodeProtocolError:
        return "Protocol Error";
    case QWebSocketProtocol::CloseCodeDatatypeNotSupported:
        return "Unsupported Data";
    case QWebSocketProtocol::CloseCodeReserved1004:
        return "No Status Received";
    case QWebSocketProtocol::CloseCodeMissingStatusCode:
        return "Missing Status";
    case QWebSocketProtocol::CloseCodeAbnormalDisconnection:
        return "Abnormal Disconnection";
    case QWebSocketProtocol::CloseCodeWrongDatatype:
        return "Wrong Data type";
    case QWebSocketProtocol::CloseCodePolicyViolated:
        return "Policy Violation";
    case QWebSocketProtocol::CloseCodeTooMuchData:
        return "Message Too Big";
    case QWebSocketProtocol::CloseCodeMissingExtension:
        return "Missing Extension";
    case QWebSocketProtocol::CloseCodeBadOperation:
        return "Bad Operation";
    case QWebSocketProtocol::CloseCodeTlsHandshakeFailed:
        return "TLS Handshake Failure";
    default:
        return QString("Unknown (%1)").arg(static_cast<int>(code));
    }
}


double MyFunc::calculateFSPL(double distanceMeters, double frequencyHz) {
    // distanceMeters: m
    // frequencyHz: Hz
    // const double speedOfLight = 3e8; // m/s
    double fspl = 20 * qLn(distanceMeters) / qLn(10)
                  + 20 * qLn(frequencyHz) / qLn(10)
                  - 147.55;
    return fspl;
}
double MyFunc::euclideanDistance(double az1, double el1, double az2, double el2) {
    //歐氏距離
    return std::sqrt(std::pow(az1 - az2, 2) + std::pow(el1 - el2, 2));
}

QStringList MyFunc::getAllIPAddress(bool onlyIPv4)
{
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    QStringList ds;
    for (const QNetworkInterface &interface : interfaces) {
        // Skip down or loopback interfaces
        if (!(interface.flags() & QNetworkInterface::IsUp) ||
            (interface.flags() & QNetworkInterface::IsLoopBack)){
            continue;
        }

        for (const QNetworkAddressEntry &entry : interface.addressEntries()) {
            QHostAddress ip = entry.ip();
            if (onlyIPv4){
                if (ip.protocol() == QAbstractSocket::IPv4Protocol){
                    ds.append(ip.toString());
                }
            }else{
                ds.append(ip.toString());
            }
        }
    }
    return ds;
}
#if defined(Q_OS_WIN)
QString MyFunc::getErrorString(DWORD errorCode)
{
    LPVOID msgBuffer;
    DWORD size = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&msgBuffer,
        0,
        nullptr
        );

    QString errorMsg;
    if (size && msgBuffer) {
        errorMsg = QString::fromWCharArray((wchar_t*)msgBuffer).trimmed();
        LocalFree(msgBuffer);
    } else {
        errorMsg = QString("Unknown error code: %1").arg(errorCode);
    }

    return errorMsg;
}
#endif
