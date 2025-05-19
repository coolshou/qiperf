#include "virtualdevice.h"

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
    //"No implementation"
    Q_UNUSED(data)
}
