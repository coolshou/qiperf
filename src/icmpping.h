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
    explicit IcmpPing(QString target, int count=4, bool isV4=true, QObject *parent = nullptr);


signals:
    void pingSuccess(int responseTime);
    void pingFailed();
private:

    // IPv4/IPv6
    // buffer
    QThread *m_thread;
    IcmpWrapper *m_icmpwapper;

};

#endif // ICMPPING_H
