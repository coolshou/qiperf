#ifndef UDPSRV_H
#define UDPSRV_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include "myinfo.h"

class UdpSrv : public QObject
{
    Q_OBJECT
public:
    explicit UdpSrv(quint16 port, QString mgr_ifname, MyInfo *myinfo,  QObject *parent = nullptr);

public slots:
    void readyRead();
    void onTimeout();
    void update_addr();
    void setSendMsg(QString msg);

signals:
private:
    quint16 m_port;
    QUdpSocket *socket;
    QTimer *infomer;
    QString m_sendMsg="";
    QString m_ifname; //manager interface name
    QHostAddress m_addr;
    QHostAddress m_baddr; //broadcast addr
    MyInfo *m_myinfo;
};

#endif // UDPSRV_H
