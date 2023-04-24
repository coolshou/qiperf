#include "pipeserver.h"

#include <QDebug>

PipeServer::PipeServer(QString servername, QObject *parent)
    : QObject{parent}
{
    m_server = 0;
    m_servername = servername;
}

PipeServer::~PipeServer()
{
    if (m_server){
        m_server->close();
        delete m_server;
    }
}

int PipeServer::isServerRun()
{
    // 用一個localsocket去連一下,如果能連上就說明 有一個在運行了
    QLocalSocket ls;
    ls.connectToServer(m_servername);
    if (ls.waitForConnected(1000)){
        ls.disconnectFromServer();// 說明已經在運行了
        ls.close();
        return 1;
    }
    return 0;
}

int PipeServer::init()
{
    // 如果已經有一個實例在運行了就返回0
    if (isServerRun()) {
        qDebug() << "another server running" << Qt::endl;
        return 1;
    }
    m_server = new QLocalServer;
    // 先移除原來存在的,如果不移除那麼如果
    // servername已經存在就會listen失敗
    QLocalServer::removeServer(m_servername);
    // 進行監聽
    qDebug() << "start PipeServer listen on: " << m_servername << Qt::endl;
    m_server->setSocketOptions(QLocalServer::WorldAccessOption);
    m_server->listen(m_servername);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(socket_new_connection()));
    return 0;
}

void PipeServer::socket_new_connection()
{   //when new client connect
    QLocalSocket *newsocket = m_server->nextPendingConnection();
    connect(newsocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(newsocket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));

}

void PipeServer::readyRead()
{
    // 取得是哪個 localsocket 可以讀數據了
    QLocalSocket *local = static_cast<QLocalSocket *>(sender());
    if (!local) return;
    m_locals.append(local);
    int idx = m_locals.count()-1;
    QDataStream in(local);
    QString     readMsg;
    in >> readMsg;// 讀出數據

    emit newMessage(idx, readMsg);// 發送收到數據信號
}

void PipeServer::on_disconnected()
{
    QLocalSocket *local = static_cast<QLocalSocket *>(sender());
    if (!local) return;
    int idx = m_locals.indexOf(local);
    if ( idx !=-1){
        m_locals.removeAt(idx);
    }
}

void PipeServer::send_MessageBack(int idx, QString message)
{
    if (m_locals.count()> idx){
        QLocalSocket *socket = m_locals[idx];
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_15);
        out << message;
        out.device()->seek(0);
        socket->write(block);
        socket->flush();
    } else{
        qDebug() << "send_MessageBack: idx " << idx << " out of range: " << m_locals.count() << Qt::endl;
    }
}
