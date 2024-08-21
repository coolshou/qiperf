#ifndef MYFUNC_H
#define MYFUNC_H

#include <QHostAddress>
#include <QAbstractSocket>

bool isValidIpAddress(const QString &ip, int &protocal) {
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


#endif // MYFUNC_H
