#ifndef MYSERVICE_H
#define MYSERVICE_H

#include <QObject>
#include <jcon/json_rpc_websocket_server.h>

class MyService : public QObject
{
    Q_OBJECT
public:
    explicit MyService(QObject *parent = nullptr);
    Q_INVOKABLE QString getOS();

    void setManagerInterface(QString interface);
    QString getManagerInterface();

signals:

private:
//    static auto rpc_server;
    QString m_interface;
};

#endif // MYSERVICE_H
