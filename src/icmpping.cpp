#include "icmpping.h"

//#if defined(Q_OS_WIN32)
//#include "winsock2.h"
//#include "iphlpapi.h"
//#include "icmpapi.h"
//#endif

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <QDebug>

IcmpPing::IcmpPing(QString refrow, QString config, QObject *parent)
    : QObject{parent},m_refrow(refrow)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(config.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        QJsonObject jobj = doc.object();
        m_target = jobj.value("target").toString();
        m_count = jobj.value("count").toInt();
        m_timeout = jobj.value("timeout").toInt();
        m_interval = jobj.value("interval").toInt();
        m_packetsize = jobj.value("packetsize").toInt();
        m_source = jobj.value("source").toString(); // TODO: ping from source
        m_ttl = jobj.value("ttl").toInt();
//        IcmpPing(refrow, target, count, timeout, interval, packetsize , source, parent);
        init(m_target, m_count, m_timeout, m_interval, m_packetsize, m_source, m_ttl);

    }else{
        qDebug() << QString("CMD_PING: ERROR: %1\nparser json: %2").arg(error.errorString())
                    .arg(config);
    }
}

IcmpPing::IcmpPing(QString refrow, QString target, int count,uint64_t timeout,
                   uint interval, uint packetsize, QString source, int ttl,
                   QObject *parent)
    : QObject{parent},m_refrow(refrow),
      m_target(target), m_count(count), m_timeout(timeout),
      m_interval(interval), m_packetsize(packetsize), m_source(source),
      m_ttl(ttl)
{
    // TODO: count -1 : continious?
    init(m_target, m_count, m_timeout, m_interval, m_packetsize, m_source, m_ttl);
}

void IcmpPing::init(QString target, int count, uint64_t timeout, uint interval,
                    uint packetsize, QString source, int ttl)
{
    int idx = m_threads.count();

    QThread *m_thread = new QThread();
    IcmpWrapper *m_icmpwapper = new IcmpWrapper(idx, target , count , timeout,
                                                interval, packetsize, source,
                                                ttl);
    connect(m_icmpwapper, &IcmpWrapper::finished, this, &IcmpPing::onFinished);
    connect(m_icmpwapper, &IcmpWrapper::icmpResponseTime, this, &IcmpPing::onResponseTime);
    connect(m_icmpwapper, &IcmpWrapper::icmpResponse, [](const QString& message){
        qDebug() << message;
    });
    //connect(m_thread, &QThread::started, m_icmpwapper, &IcmpWrapper::start);
    connect(m_thread, &QThread::started, m_icmpwapper, &IcmpWrapper::work);

    connect(m_thread, &QThread::finished, m_icmpwapper, &IcmpWrapper::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
    m_icmpwapper->moveToThread(m_thread);

    m_threads.insert(idx, m_thread);
//    m_pingworkers.insert(idx, m_icmpwapper);
//    qDebug() << " create Qthread count:"<< m_threads.count() << " idx:" << idx << " : " << m_threads;
}

void IcmpPing::start()
{
    if (m_threads.count()>0){
        m_threads.value(0)->start();
    }
}

void IcmpPing::onFinished(int idx)
{
    qDebug() << "onFinished" << idx;
    qDebug() << "Ping " << m_target << " Finished";
    if (m_threads.contains(idx)){
        m_threads.take(idx);
    }
}

void IcmpPing::onResponseTime(uint16_t seq, double responseTime, const char *checksum)
{
    emit icmpResponseTime(m_refrow, seq, responseTime, checksum);
}



