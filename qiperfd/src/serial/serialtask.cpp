#include "serialtask.h"

#include <QTimer>

SerialTask::SerialTask(QString midx, const QString& serialPortName, const QString& serialBaudRate,
                       const QString& localIp, const QString& localPort,
                       VirtualDeviceTcp::Mode mode,
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
    , _DeviceTcp(0)
    // , _comDeviceScreen(0)
{
    _lasterror = "";
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

bool SerialTask::isRunning()
{
    if (_comDeviceSerial && _DeviceTcp){
        if (_comDeviceSerial->isRunning() && _DeviceTcp->isRunning()){
            return true;
        }else{
            if (!_comDeviceSerial->isRunning()){
                _lasterror = "_comDeviceSerial not running";
                qDebug() << _lasterror;
            }
            if (!_DeviceTcp->isRunning()){
                _lasterror = _lasterror + " _DeviceTcp not running";
                qDebug() << _lasterror;
            }
            return false;
        }
    }else {
        _lasterror = "_comDeviceSerial or _DeviceTcp not exist";
        return false;
    }
}

QString SerialTask::getLastError()
{
    return _lasterror;
}

void SerialTask::setConfig(QString localPort, QString serialBaudRate,
                           QSerialPort::DataBits serialDataBits,
                           QSerialPort::Parity serialParity,
                           QSerialPort::StopBits serialStopBits,
                           QSerialPort::FlowControl serialFlowControl)
{
    _localPort = localPort;
    _serialBaudRate = serialBaudRate;
    _serialDataBits = serialDataBits;
    _serialParity = serialParity;
    _serialStopBits = serialStopBits;
    _serialFlowControl = serialFlowControl;
}

void SerialTask::init()
{
    _comDeviceSerial = new ComDeviceSerial(_serialPortName, _serialBaudRate, this,
                                           _serialDataBits, _serialParity,
                                           _serialStopBits, _serialFlowControl);
    _DeviceTcp = new VirtualDeviceTcp(m_idx, _localIp, _localPort, _mode, this);
    // _comDeviceScreen = new ComDeviceScreen(this);

    connect(_comDeviceSerial, &VirtualDevice::finished, this, &SerialTask::slotFinished);
    connect(_DeviceTcp, &VirtualDevice::finished, this, &SerialTask::slotFinished);
    connect(_DeviceTcp, &VirtualDeviceTcp::started, this, &SerialTask::onStarted);
    // connect(_comDeviceScreen, &ComDevice::finished, this, &SerialTask::slotFinished);

    connect(_comDeviceSerial, &VirtualDevice::signalDataRecv, _DeviceTcp, &VirtualDevice::slotDataSend);
    connect(_DeviceTcp, &VirtualDevice::signalDataRecv, _comDeviceSerial, &VirtualDevice::slotDataSend);
    // if (_localOutput) {
    //     connect(_comDeviceSerial, &VirtualDevice::signalDataRecv, _comDeviceScreen, &VirtualDevice::slotDataSend);
    // }
    // if (_localInput) {
    //     connect(_comDeviceScreen, &VirtualDevice::signalDataRecv, _comDeviceSerial, &VirtualDevice::slotDataSend);
    // }

    QTimer::singleShot(0, _comDeviceSerial, SLOT(init()));
    QTimer::singleShot(0, _DeviceTcp, SLOT(init()));
    // QTimer::singleShot(0, _comDeviceScreen, SLOT(init()));
}

void SerialTask::close()
{
    if(_comDeviceSerial){
        _comDeviceSerial->close();
    }
    if(_DeviceTcp){
        _DeviceTcp->close();
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
