#ifndef RPCTP_H
#define RPCTP_H

#include <QObject>
#include "jcon/json_rpc_websocket_client.h"
#include "tp.h"

class RpcTp : public QObject
{
    Q_OBJECT
//    Q_PROPERTY(jcon::JsonRpcWebSocketClient *rpc READ getRPC WRITE setRPC )
//    Q_PROPERTY(TP *tp; READ getTP WRITE setTP )

public:
    explicit RpcTp(QObject *parent = nullptr);
    jcon::JsonRpcWebSocketClient* getRPC();
    void setRPC(const jcon::JsonRpcWebSocketClient &rpc);
    TP* getTP();
    void setTP(const TP &tp);

signals:
private:
    jcon::JsonRpcWebSocketClient *m_rpc;
    TP *m_tp;
};

#endif // RPCTP_H
