#include "virtualdevice.h"

#include <QDebug>

VirtualDevice::VirtualDevice(QObject *parent)
    : QObject{parent}
{}

VirtualDevice::~VirtualDevice()
{

}

void VirtualDevice::init()
{
    //"No implementation"
}

void VirtualDevice::slotDataSend(const QByteArray &data)
{
    qDebug() << "VirtualDevice::slotDataSend No implementation";
    Q_UNUSED(data)
}
