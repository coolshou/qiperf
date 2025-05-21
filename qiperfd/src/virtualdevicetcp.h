#ifndef VIRTUALDEVICETCP_H
#define VIRTUALDEVICETCP_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#include "virtualdevice.h"

class VirtualDeviceTcp : public VirtualDevice
{
    Q_OBJECT
public:
    enum class Mode { TEXT, BINARY };
    explicit VirtualDeviceTcp(QString idx, const QString& localIp, const QString& localPort,
                          Mode mode=Mode::BINARY,
                          QObject *parent = nullptr);
    virtual ~VirtualDeviceTcp();
    bool isRunning();

signals:
    void started(QString idx, quint16 port);// notice run on which port

public slots:
    void init() override;
    void slotDataSend(const QByteArray& data) override;
    void close();
    void slotAcceptError(QAbstractSocket::SocketError socketError);
    void slotNewConnection();
    void slotDisconnected();
    void slotReadyRead();

private:
    Q_DISABLE_COPY(VirtualDeviceTcp)
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

#endif // VIRTUALDEVICETCP_H
