#ifndef PIPECLIENT_H
#define PIPECLIENT_H

#include <QObject>
#include <QLocalSocket>
#include <QCoreApplication>
#include <QString>
#include <QDataStream>
#include <QTimer>

class PipeClient : public QObject
{
    Q_OBJECT
public:
    explicit PipeClient(QString remoteServername, QObject *parent = nullptr);
    ~PipeClient() override;
    void SetAppHandle( QCoreApplication *app);

signals:
    void newMessage(const QString &msg);
    void sigError(QString msg);

public slots:
    void send_MessageToServer(QString message);

    void socket_connected();
    void socket_disconnected();

    void socket_readReady();
    void socket_error(QLocalSocket::LocalSocketError err);

private:
    QLocalSocket*   m_socket;
    quint16         m_blockSize;
    QString         m_message;
    QString         m_serverName;

    QCoreApplication * m_AppHandle;
};

#endif // PIPECLIENT_H
