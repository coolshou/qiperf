#ifndef MYSERVICE_H
#define MYSERVICE_H

#include <QObject>
#if (TEST_JSONRPC==1)
#include <jcon/json_rpc_websocket_server.h>
#endif
#include "qiperfd.h"

class MyService : public QObject
{
    Q_OBJECT
public:
    explicit MyService(QIperfd* qiperfd, QObject *parent = nullptr);
    Q_INVOKABLE QString getOS();
    Q_INVOKABLE int addIperfServer(QString refrow, int version, uint port, QString bindHost="");
    Q_INVOKABLE int addIperfClient(QString refrow, int version, uint port, QString Host, QString iperfargs);
    Q_INVOKABLE void start(int idx);
    Q_INVOKABLE void startAll();
    Q_INVOKABLE void stop(int idx);
    Q_INVOKABLE void stopAll();

    void setManagerInterface(QString ifname);
    QString getManagerInterface();

signals:
    void sig_setManagerInterface(QString ifname);
private:
//    static auto rpc_server;
//    QString m_interface;
    QIperfd* m_qiperfd;
};

#endif // MYSERVICE_H
