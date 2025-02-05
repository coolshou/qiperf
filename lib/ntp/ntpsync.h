#ifndef NTPSYNC_H
#define NTPSYNC_H

#include <QObject>
#include <QDateTime>
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
    void setSystemTime(const QDateTime &dateTime);
    void sync(QString server);
public slots:
    void onReplyReceived(const QHostAddress &address, quint16 port, const NtpReply &reply);
signals:
private:
    NtpClient *m_ntpclient;

};

#endif // NTPSYNC_H
