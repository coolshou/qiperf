#include "icmpping.h"

#if defined(Q_OS_WIN32)
#include "winsock2.h"
#include "iphlpapi.h"
#include "icmpapi.h"
#endif


#include <QDebug>

IcmpPing::IcmpPing(QString target, int count, bool isV4, QObject *parent)
    : QObject{parent}
{
//QString h;
//h.toStdString().
    // TODO: count -1 : continious?
    m_thread = new QThread();
    m_icmpwapper = new IcmpWrapper(target , count , isV4);
    connect(m_thread, &QThread::started, m_icmpwapper, &IcmpWrapper::start);
    // connect(m_icmpwapper, &IcmpWrapper::finished, m_thread, &QThread::);
    m_icmpwapper->moveToThread(m_thread);

}


