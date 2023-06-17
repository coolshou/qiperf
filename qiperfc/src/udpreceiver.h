#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include <QObject>
#include <QUdpSocket>
#include "../src/comm.h"

class UdpReceiver : public QObject
{
    Q_OBJECT
public:
    explicit UdpReceiver(quint16 port, QObject *parent = nullptr);
signals:
    void notice(QString send_addr, QString msg); //send_addr, msg: json format info

public slots:
    void dataReceived();


private:
    QUdpSocket *m_socket;
};

#endif // UDPRECEIVER_H
