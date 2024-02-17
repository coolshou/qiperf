#ifndef PIPESERVER_H
#define PIPESERVER_H

/*
 * class pipeserver
 *  a QLocalServer allow app to run only once
 *  and accept second run time's args list
*/
#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>
#include <QStringList>

class PipeServer : public QObject
{
    Q_OBJECT
public:
    explicit PipeServer(QString servername, qint64 pid, QObject *parent = nullptr);
    ~PipeServer() override;
    int isServerRun();
    int init();
    void sendARGS(QStringList args);
    inline void delay(int millisecondsWait);

signals:
    void messageReceived(QString);
    void pipeMessage(int idx, const QString msg);

public slots:
    void socket_new_connection();
    void readyRead();
    void on_disconnected();
    void send_MessageBack(int idx, QString message);

private:
    qint64 m_pid;
    QString m_servername;
    QLocalServer *m_server;
    QLocalSocket *m_socket;
    QList<QLocalSocket*> m_locals; //client sockets list

};

#endif // PIPESERVER_H
