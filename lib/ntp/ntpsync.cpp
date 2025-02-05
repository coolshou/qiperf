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
}

void NtpSync::sync(QString server)
{
    QHostAddress address;// = QHostAddress(server);
    if (address.setAddress(server)){
        //server: ip address
        if (!m_ntpclient->sendRequest(address, 123)) {
            qDebug() << "Failed to send NTP request: " << m_ntpclient->socket()->errorString();
        }
    }else {
        qDebug() << "Fail to set QHostAddress with " << server ;
    }
}

void NtpSync::onReplyReceived(const QHostAddress &address, quint16 port, const NtpReply &reply)
{
    // qDebug() << "Reply received from " << address.toString() << ":" << QString::number(port) << ": {\n"
    //        << "    leapIndicator    = " << QString::number(reply.leapIndicator()) << "\n"
    //        << "    versionNumber    = " << QString::number(reply.versionNumber()) << "\n"
    //        << "    mode             = " << QString::number(reply.mode()) << "\n"
    //        << "    stratum          = " << QString::number(reply.stratum()) << "\n"
    //        << "    pollInterval     = " << QString::number(reply.pollInterval()) << "s\n"
    //        << "    precision        = " << QString::number(reply.precision(), 10, 15) << "s\n"
    //        << "    referenceTime    = " << reply.referenceTime().toString(Qt::ISODateWithMs) << "\n"
    //        << "    originTime       = " << reply.originTime().toString(Qt::ISODateWithMs) << "\n"
    //        << "    receiveTime      = " << reply.receiveTime().toString(Qt::ISODateWithMs) << "\n"
    //        << "    transmitTime     = " << reply.transmitTime().toString(Qt::ISODateWithMs) << "\n"
    //        << "    destinationTime  = " << reply.destinationTime().toString(Qt::ISODateWithMs) << "\n"
    //        << "    roundTripDelay   = " << QString::number(reply.roundTripDelay()) << "ms\n"
    //        << "    localClockOffset = " << QString::number(reply.localClockOffset()) << "ms\n"
    //        << "}\n";
    setSystemTime(reply.destinationTime());
}
