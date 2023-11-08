#ifndef MYSERVICE_H
#define MYSERVICE_H

#include <QObject>
#include <jcon/json_rpc_websocket_server.h>
#include "qiperfd.h"

class MyService : public QObject
{
    Q_OBJECT
public:
    explicit MyService(QIperfd* qiperfd, QObject *parent = nullptr);
    Q_INVOKABLE QString getOS();
    Q_INVOKABLE int addIperfServer(int version, uint port, QString bindHost="");
    Q_INVOKABLE int addIperfClient(int version, uint port, QString Host, QString iperfargs);
    Q_INVOKABLE void start(int idx);
    Q_INVOKABLE void startAll();
    Q_INVOKABLE void stop(int idx);
    Q_INVOKABLE void stopAll();

    void setManagerInterface(QString interface);
    QString getManagerInterface();

signals:
    void sig_setManagerInterface(QString interface);
private:
//    static auto rpc_server;
//    QString m_interface;
    QIperfd* m_qiperfd;
};

#endif // MYSERVICE_H
