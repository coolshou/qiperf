#include "myservice.h"
#include <QSysInfo>

#include <QDebug>

MyService::MyService(QIperfd *qiperfd, QObject *parent)
    : QObject{parent}
{
    m_qiperfd = qiperfd;
//    m_interface="";
}

QString MyService::getOS()
{
    qDebug() << "getOS \n" << Qt::endl;
    return QSysInfo::productType();
}

int MyService::addIperfServer(int version, uint port, QString bindHost)
{
    return m_qiperfd->addIperfServer(version, port, bindHost);
}

int MyService::addIperfClient(int version, uint port, QString Host, QString iperfargs)
{
    return m_qiperfd->addIperfClient(version, port, Host, iperfargs);
}

void MyService::start(int idx)
{
    return m_qiperfd->start(idx);
}

void MyService::startAll()
{
    return m_qiperfd->startAll();
}

void MyService::stop(int idx)
{
    return m_qiperfd->stop(idx);
}

void MyService::stopAll()
{
    return m_qiperfd->stopAll();
}

void MyService::setManagerInterface(QString interface)
{
//    m_interface = interface;
   emit sig_setManagerInterface(interface);
}

QString MyService::getManagerInterface()
{
   return m_qiperfd->getManagerInterface();
//    return m_interface;
}
