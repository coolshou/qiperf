#ifndef AGENT_H
#define AGENT_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QNetworkInterface>
#include <QNetworkDatagram>

#include <QDebug>

class Agent : public QObject
{
    Q_OBJECT
public:
    explicit Agent(QObject *parent = nullptr,
                   const QHostAddress& outgoingIP = QHostAddress::LocalHost,
                   unsigned int outgoingPort = 45454,
                   bool acceptmsg = false);

public slots:
    void sendMessage(QString message);
    void updateIpaddress(QString ipaddress, int port);
    void processData();
signals:
    void receivedMessage(QString message);

protected:
    void initialize();

    QUdpSocket *udpSocket;
    QHostAddress outputIPAddress;
//    unsigned int inputPort;
    unsigned int outputPort;

private slots:
    void broadcastDatagram();
    bool isValidNetworkAdapter(const QNetworkInterface &adapter);
    bool isValidNetworkAdapter(QString adapterName);
    QNetworkInterface getNetworkAdapter(QString adapterName);

private:
    bool m_acceptmsg;
    QTimer *timer;
};

#endif // AGENT_H
