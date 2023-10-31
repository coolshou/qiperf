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

void MyService::start()
{
    return m_qiperfd->start();
}

void MyService::stop()
{
    return m_qiperfd->stop();
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
