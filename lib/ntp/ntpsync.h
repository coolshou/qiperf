#ifndef NTPSYNC_H
#define NTPSYNC_H

#include <QObject>
#include <QDateTime>
#include <QHostAddress>

#if defined(Q_OS_WIN32)
#include <windows.h>
#endif
#if defined(Q_OS_LINUX)
#include <sys/time.h>
#endif

#include "../lib/qntp/src/qntp/NtpClient.h"

class NtpSync : public QObject
{
    Q_OBJECT
public:
    explicit NtpSync(QObject *parent = nullptr);
    bool setSystemTime(const QDateTime &dateTime);
    void sync(QString server);
    void sync(QHostAddress server);
public slots:
    void onReplyReceived(const QHostAddress &address, quint16 port, const NtpReply &reply);
signals:
    void timesynced(QString target, bool synced);

private:
    NtpClient *m_ntpclient;

};

#endif // NTPSYNC_H
