#ifndef NTPSERVER_H
#define NTPSERVER_H

#include <QObject>
#include <QtNetwork>
#include <QDateTime>
#include <QUdpSocket>

class NtpServer : public QObject
{
    Q_OBJECT
public:
    explicit NtpServer(QObject *parent = nullptr);
    QByteArray createNTPResponse(const QByteArray &request);
public slots:
    void processPendingDatagrams();
signals:

private:
    QUdpSocket *udpSocket;
};

#endif // NTPSERVER_H
