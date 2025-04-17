#include "serialtask.h"

#include <QTimer>

SerialTask::SerialTask(QString midx, const QString& serialPortName, const QString& serialBaudRate,
                       const QString& localIp, const QString& localPort,
                       ComDeviceTcp::Mode mode,
                       QSerialPort::DataBits serialDataBits,
                       QSerialPort::Parity serialParity,
                       QSerialPort::StopBits serialStopBits,
                       QSerialPort::FlowControl serialFlowControl,
                       bool localInput, bool localOutput, QObject *parent)
    : QObject(parent)
    , m_idx(midx)
    , _serialPortName(serialPortName)
    , _serialBaudRate(serialBaudRate)
    , _localIp(localIp)
    , _localPort(localPort)
    , _mode(mode)
    , _serialDataBits(serialDataBits)
    , _serialParity(serialParity)
    , _serialStopBits(serialStopBits)
    , _serialFlowControl(serialFlowControl)
    , _localInput(localInput)
    , _localOutput(localOutput)
    , _comDeviceSerial(0)
    , _comDeviceTcp(0)
    // , _comDeviceScreen(0)
{

}

SerialTask::~SerialTask()
{

}

quint16 SerialTask::getLocalPort()
{
    return _localPort.toUInt();
}

QString SerialTask::getIdx()
{
    return m_idx;
}

void SerialTask::init()
{
    _comDeviceSerial = new ComDeviceSerial(_serialPortName, _serialBaudRate, this,
                                           _serialDataBits, _serialParity,
                                           _serialStopBits, _serialFlowControl);
    _comDeviceTcp = new ComDeviceTcp(m_idx, _localIp, _localPort, _mode, this);
    // _comDeviceScreen = new ComDeviceScreen(this);

    connect(_comDeviceSerial, &ComDevice::finished, this, &SerialTask::slotFinished);
    connect(_comDeviceTcp, &ComDevice::finished, this, &SerialTask::slotFinished);
    connect(_comDeviceTcp, &ComDeviceTcp::started, this, &SerialTask::onStarted);
    // connect(_comDeviceScreen, &ComDevice::finished, this, &SerialTask::slotFinished);

    connect(_comDeviceSerial, &ComDevice::signalDataRecv, _comDeviceTcp, &ComDevice::slotDataSend);
    connect(_comDeviceTcp, &ComDevice::signalDataRecv, _comDeviceSerial, &ComDevice::slotDataSend);
    // if (_localOutput) {
    //     connect(_comDeviceSerial, &ComDevice::signalDataRecv, _comDeviceScreen, &ComDevice::slotDataSend);
    // }
    // if (_localInput) {
    //     connect(_comDeviceScreen, &ComDevice::signalDataRecv, _comDeviceSerial, &ComDevice::slotDataSend);
    // }

    QTimer::singleShot(0, _comDeviceSerial, SLOT(init()));
    QTimer::singleShot(0, _comDeviceTcp, SLOT(init()));
    // QTimer::singleShot(0, _comDeviceScreen, SLOT(init()));
}

void SerialTask::close()
{
    if(_comDeviceSerial){
        _comDeviceSerial->close();
    }
    if(_comDeviceTcp){
        _comDeviceTcp->close();
    }
}

void SerialTask::slotFinished()
{
    emit finished(_serialPortName);
}

void SerialTask::onStarted(QString idx, quint16 port)
{
    emit started(idx, port);
}
