#include "ntpsync.h"

#include <QDebug>

NtpSync::NtpSync(QObject *parent)
    : QObject{parent}
{
    m_ntpclient = new NtpClient(this);
    if(!m_ntpclient->bind()) {
        qDebug() << "Couldn't bind socket: " << m_ntpclient->socket()->errorString();
        // return 1;
    }
    connect(m_ntpclient, &NtpClient::replyReceived, this, &NtpSync::onReplyReceived);
}

void NtpSync::setSystemTime(const QDateTime &dateTime)
{
    qDebug() << "Set system time to: " << dateTime.toString(Qt::ISODateWithMs);
#if defined(Q_OS_WIN32)
    SYSTEMTIME st;
    QDate date = dateTime.date();
    QTime time = dateTime.time();

    st.wYear = date.year();
    st.wMonth = date.month();
    st.wDay = date.day();
    st.wHour = time.hour();
    st.wMinute = time.minute();
    st.wSecond = time.second();
    st.wMilliseconds = time.msec();

    // Set the system time
    if (!SetSystemTime(&st)) {
        qWarning("Failed to set system time");
    }
#endif
#if defined(Q_OS_LINUX)
    struct timeval tv;
    QDate date = dateTime.date();
    QTime time = dateTime.time();

    struct tm newTime;
    newTime.tm_year = date.year() - 1900; // Years since 1900
    newTime.tm_mon = date.month() - 1; // Months are 0-based
    newTime.tm_mday = date.day();
    newTime.tm_hour = time.hour();
    newTime.tm_min = time.minute();
    newTime.tm_sec = time.second();
    newTime.tm_isdst = -1; // Not considering daylight saving time

    tv.tv_sec = mktime(&newTime);
    tv.tv_usec = time.msec() * 1000; // Convert milliseconds to microseconds

    if (settimeofday(&tv, NULL) != 0) {
        perror("settimeofday");
    }
#endif
    QDateTime n = QDateTime::currentDateTime();
    qint64 difftime = n.msecsTo(dateTime);
    qDebug() << "After set system time, time diff: " << QString::number(difftime) << " ms";
}

void NtpSync::sync(QString server)
{
    QHostAddress address;// = QHostAddress(server);
    if (address.setAddress(server)){
        //server: ip address
        sync(address);
        // if (!m_ntpclient->sendRequest(address, 123)) {
        //     qDebug() << "Failed to send NTP request: " << m_ntpclient->socket()->errorString();
        // }
    }else {
        qDebug() << "Fail to set QHostAddress with " << server ;
    }
}

void NtpSync::sync(QHostAddress server)
{
    if (!m_ntpclient->sendRequest(server, 123)) {
        qDebug() << "Failed to send NTP request: " << m_ntpclient->socket()->errorString();
    }
}

void NtpSync::onReplyReceived(const QHostAddress &address, quint16 port, const NtpReply &reply)
{
    qDebug() << "NtpSync onReplyReceived:" << address.toString() << " port:" << QString::number(port);
    //qDebug() << "originTime:" << reply.originTime();
    qDebug() << "receiveTime:" << reply.receiveTime();
    //qDebug() << "transmitTime:" << reply.transmitTime();
    //qDebug() << "destinationTime:" << reply.destinationTime();

    setSystemTime(reply.receiveTime());
}
