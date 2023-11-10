#include "rpctp.h"

RpcTp::RpcTp(QObject *parent)
    : QObject{parent}
{

}

jcon::JsonRpcWebSocketClient* RpcTp::getRPC()
{
    return m_rpc;
}

void RpcTp::setRPC(const jcon::JsonRpcWebSocketClient &rpc)
{
    Q_UNUSED(rpc)
//    m_rpc = rpc;
}

TP* RpcTp::getTP()
{
    return m_tp;
}

void RpcTp::setTP(const TP &tp)
{
    Q_UNUSED(tp)
//    m_tp = tp;
}
