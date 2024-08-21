#ifndef ICMPPING_H
#define ICMPPING_H

#include <QObject>
#include <QThread>

#include "icmpwrapper.h"


#if defined(Q_OS_LINUX)
// #include <oping.h>
#endif

class IcmpPing : public QObject
{
    Q_OBJECT
public:
    explicit IcmpPing(QString refrow, QString config, QObject *parent = nullptr);
    explicit IcmpPing(QString refrow, QString target, int count=4, uint64_t timeout=3,
                      uint interval=1, uint packetsize=64, QString source="", int ttl=64,
                      QObject *parent = nullptr);
    void start();

public slots:
//    void onTTL(uint16_t seq, int ttl);
    void onFinished(int idx);
signals:
    void pingSuccess(int responseTime);
    void pingFailed();

private:
    void init(QString target, int count=4, uint64_t timeout=3,
              uint interval=1, uint packetsize=64, QString source="",
              int ttl=64);
    QString m_refrow;
    QString m_target;
    quint64 m_count; // max ping
    uint64_t m_timeout; // TODO
    uint m_interval; // TODO
    uint m_packetsize; // TODO
    QString m_source; // TODO
    uint m_ttl;
    QMap<int, IcmpWrapper*> m_pingworkers;
    QMap<int, QThread*> m_threads; // iperfworker's thread
};

#endif // ICMPPING_H
