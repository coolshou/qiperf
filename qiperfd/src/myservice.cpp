#include "myservice.h"
#include <QSysInfo>

#include <QDebug>

MyService::MyService(QObject *parent)
    : QObject{parent}
{
    m_interface="";
}

QString MyService::getOS()
{
    qDebug() << "getOS \n" << Qt::endl;
    return QSysInfo::productType();
}

void MyService::setManagerInterface(QString interface)
{
    m_interface = interface;
}

QString MyService::getManagerInterface()
{
    return m_interface;
}
