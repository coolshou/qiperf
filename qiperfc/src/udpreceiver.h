#ifndef UDPRECEIVER_H
#define UDPRECEIVER_H

#include <QObject>
#include <QUdpSocket>
#include "../src/comm.h"

class UdpReceiver : public QObject
{
    Q_OBJECT
public:
    explicit UdpReceiver(int port, QObject *parent = nullptr);
signals:
    void notice(QString msg);

public slots:
    void dataReceived();


private:
    QUdpSocket *m_socket;
};

#endif // UDPRECEIVER_H
