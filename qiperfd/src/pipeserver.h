#ifndef PIPESERVER_H
#define PIPESERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>

class PipeServer : public QObject
{
    Q_OBJECT
public:
    explicit PipeServer(QString servername, QObject *parent = nullptr);
    ~PipeServer();
    int isServerRun();
    int init();

signals:
    void messageReceived(QString);
    void newMessage(int idx, const QString &msg);

public slots:
    void socket_new_connection();
    void readyRead();
    void on_disconnected();
    void send_MessageBack(int idx, QString message);

private:
    QString m_servername;
    QLocalServer *m_server;
    QLocalSocket *m_socket;
    QList<QLocalSocket*> m_locals; //client sockets list

};

#endif // PIPESERVER_H
