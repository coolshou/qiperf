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
