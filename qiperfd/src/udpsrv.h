#ifndef UDPSRV_H
#define UDPSRV_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>

class UdpSrv : public QObject
{
    Q_OBJECT
public:
    explicit UdpSrv(int port, QString mgr_ifname, QObject *parent = nullptr);
    QString collectInfo();

public slots:
    void readyRead();
    void onTimeout();
signals:
private:
    int m_port;
    QUdpSocket *socket;
    QTimer *infomer;
    QString sendMsg="";
    QString m_ifname; //manager interface name
};

#endif // UDPSRV_H
