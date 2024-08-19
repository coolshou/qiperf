#include "icmpwrapper.h"

#include <QDebug>
IcmpWrapper::IcmpWrapper(QString target, quint64 count, bool isV4, QObject *parent)
    : QObject{parent},
    m_target(target), m_count(count), m_isV4(isV4)
{
    m_idx = 0;
    connect(this, &IcmpWrapper::finished, this, &IcmpWrapper::stop);
}

void IcmpWrapper::start()
{    //run in thread
    m_idx = 0;
    qDebug() << "start";
    for(m_idx=0; m_idx < m_count; m_idx++){
        pingHost(m_target, m_idx);
    }
    emit stop();
}

void IcmpWrapper::stop()
{
    qDebug() << "stop";
}

bool IcmpWrapper::pingHost(const QString &hostname, quint64 idx)
{

}
