#ifndef RPCTP_H
#define RPCTP_H

#include <QObject>
#if (TEST_JSONRPC==1)
#include "jcon/json_rpc_websocket_client.h"
#endif
#include "tp.h"

class RpcTp : public QObject
{
    Q_OBJECT
//    Q_PROPERTY(jcon::JsonRpcWebSocketClient *rpc READ getRPC WRITE setRPC )
//    Q_PROPERTY(TP *tp; READ getTP WRITE setTP )

public:
    explicit RpcTp(QObject *parent = nullptr);
#if (TEST_JSONRPC==1)
    jcon::JsonRpcWebSocketClient* getRPC();
    void setRPC(const jcon::JsonRpcWebSocketClient &rpc);
#endif
    TP* getTP();
    void setTP(const TP &tp);

signals:
private:
#if (TEST_JSONRPC==1)
    jcon::JsonRpcWebSocketClient *m_rpc;
#endif
    TP *m_tp;
};

#endif // RPCTP_H
