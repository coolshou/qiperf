#ifndef COMDEVICETCP_H
#define COMDEVICETCP_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "comdevice.h"

class ComDeviceTcp : public ComDevice
{
    Q_OBJECT
public:
    enum class Mode { TEXT, BINARY };
    explicit ComDeviceTcp(QString idx, const QString& localIp, const QString& localPort,
                          Mode mode=Mode::BINARY,
                          QObject *parent = nullptr);
    virtual ~ComDeviceTcp();
signals:
    void started(QString idx, quint16 port);// notice run on which port

public slots:
    virtual void init();
    virtual void slotDataSend(const QByteArray& data);
    void close();
    void slotAcceptError(QAbstractSocket::SocketError socketError);
    void slotNewConnection();
    void slotDisconnected();
    void slotReadyRead();

private:
    Q_DISABLE_COPY(ComDeviceTcp)

    QString peerString(const QTcpSocket* tcpSocket);

private:
    QString m_idx;
    const QString _localIp;
    const QString _localPort;
    const Mode _mode;

    QTcpServer* _tcpServer;
    QList<QTcpSocket*> _tcpSocketList;
    QMap<QString, QString> _dataRecv; // used if Mode::TEXT
};

#endif // COMDEVICETCP_H
