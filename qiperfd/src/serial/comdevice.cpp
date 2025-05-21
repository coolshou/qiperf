#include "comdevice.h"

ComDevice::ComDevice(QObject *parent)
    : QObject{parent}
{}

ComDevice::~ComDevice()
{

}

void ComDevice::init()
{
    //"No implementation"
}

void ComDevice::slotDataSend(const QByteArray &data)
{
    //"No implementation"
    Q_UNUSED(data)
}
