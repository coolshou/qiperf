#include "ntpserver.h"

#include <QDebug>

NtpServer::NtpServer(QObject *parent)
    : QObject{parent}
{
    udpSocket= new QUdpSocket(this);
    connect(udpSocket, &QUdpSocket::readyRead, this, &NtpServer::processPendingDatagrams);
    if (!udpSocket->bind(QHostAddress::Any, 123)){ // NTP uses port 123, require root
        qInfo() << "bind to NTP port 123 Fail!! (" << udpSocket->errorString() << ")";
    }

}

QByteArray NtpServer::createNTPResponse(const QByteArray &request)
{
    QByteArray response(48, 0); // NTP packet is 48 bytes

    // Set the version number to 4 (NTPv4) and mode to 4 (server)
    response[0] = (4 << 3)| 4; // VN (Version Number) is in the first byte, bits 3-5, Mode is in bits 0-2
    // Set the stratum value to 2 (indicating secondary reference)
    response[1] = 2;
    response[2] = 3; //Peer Polling Interval: 3 (8 seconds)
    response[3] = 0; //Peer Clock Precision: 0 (1.000000000 seconds)

    // Set the necessary fields in the NTP response
    // For simplicity, we'll just set the transmit timestamp
    QDateTime now = QDateTime::currentDateTimeUtc();
    quint32 seconds = now.toSecsSinceEpoch() + 2208988800U; // Convert to NTP epoch
    quint32 fraction = (now.time().msec() * 4294967.296); // Convert milliseconds to fraction
    //Reference Timestamp:

    //Origin Timestamp:
    // Copy the Transmit Timestamp from the request to the Origin Timestamp in the response
    for (int i = 0; i < 8; ++i) {
        response[24 + i] = request[40 + i];
    }
    //Receive Timestamp: Set the receive timestamp in the response
    response[32] = (seconds >> 24) & 0xFF;
    response[33] = (seconds >> 16) & 0xFF;
    response[34] = (seconds >> 8) & 0xFF;
    response[35] = seconds & 0xFF;
    response[36] = (fraction >> 24) & 0xFF;
    response[37] = (fraction >> 16) & 0xFF;
    response[38] = (fraction >> 8) & 0xFF;
    response[39] = fraction & 0xFF;

    //Transmit Timestamp: Set the transmit timestamp in the response
    response[40] = (seconds >> 24) & 0xFF;
    response[41] = (seconds >> 16) & 0xFF;
    response[42] = (seconds >> 8) & 0xFF;
    response[43] = seconds & 0xFF;
    response[44] = (fraction >> 24) & 0xFF;
    response[45] = (fraction >> 16) & 0xFF;
    response[46] = (fraction >> 8) & 0xFF;
    response[47] = fraction & 0xFF;

    return response;
}

void NtpServer::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        // Process the NTP request and prepare the response
        QByteArray response = createNTPResponse(datagram);
        udpSocket->writeDatagram(response, sender, senderPort);
    }
}
