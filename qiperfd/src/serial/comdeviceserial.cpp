#include "comdeviceserial.h"

#include <QDebug>

ComDeviceSerial::ComDeviceSerial(const QString& serialPortName, const QString& serialBaudRate,
                                 QObject *parent,
                                 QSerialPort::DataBits serialDataBits,
                                 QSerialPort::Parity serialParity,
                                 QSerialPort::StopBits serialStopBits,
                                 QSerialPort::FlowControl serialFlowControl
                                 )
    : ComDevice{parent}
    , _serialPortName(serialPortName)
    , _serialBaudRate(serialBaudRate)
    , _serialDataBits(serialDataBits)
    , _serialParity(serialParity)
    , _serialStopBits(serialStopBits)
    , _serialFlowControl(serialFlowControl)
    , _serialPort(0)
{}

ComDeviceSerial::~ComDeviceSerial()
{
    close();
}

void ComDeviceSerial::init()
{
    _serialPort = new QSerialPort(this);
    _serialPort->setPortName(_serialPortName);

    if (_serialBaudRate == "4800") {
        _serialPort->setBaudRate(QSerialPort::Baud4800);
    }
    else if (_serialBaudRate == "9600") {
        _serialPort->setBaudRate(QSerialPort::Baud9600);
    }
    else if (_serialBaudRate == "19200") {
        _serialPort->setBaudRate(QSerialPort::Baud19200);
    }
    else if (_serialBaudRate == "38400") {
        _serialPort->setBaudRate(QSerialPort::Baud38400);
    }
    else if (_serialBaudRate == "57600") {
        _serialPort->setBaudRate(QSerialPort::Baud57600);
    }
    else if (_serialBaudRate == "115200") {
        _serialPort->setBaudRate(QSerialPort::Baud115200);
    }
    else{
        qDebug() << " Not supported BaudRate: " << _serialBaudRate;
    }
    _serialPort->setDataBits(_serialDataBits);
    _serialPort->setParity(_serialParity);
    _serialPort->setStopBits(_serialStopBits);
    _serialPort->setFlowControl(_serialFlowControl);

    connect(_serialPort, &QSerialPort::readyRead, this, &ComDeviceSerial::slotReadyRead);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    connect(_serialPort, &QSerialPort::errorOccurred, this, &ComDeviceSerial::slotError);
#else
    connect(_serialPort, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), this, &ComDeviceSerial::slotError);
#endif

    if (!_serialPort->open(QIODevice::ReadWrite)) {
        // L_ERROR("Serial port open failed");
        qDebug() << "Serial port open failed";
        emit finished();
    }
}

void ComDeviceSerial::slotDataSend(const QByteArray &data)
{
    if (!_serialPort) {
        return;
    }

    qint64 number = _serialPort->write(data);
    if (number == -1) {
        qDebug() << "Serial port write failed";
        emit finished();
    }
    else if (number != data.size()) {
        qWarning() << "Serial port write partial data";
        emit finished();
    }
}

void ComDeviceSerial::close()
{
    if (_serialPort) {
        _serialPort->close();
    }
}

void ComDeviceSerial::slotReadyRead()
{
    if (!_serialPort) {
        return;
    }

    QByteArray data = _serialPort->readAll();
    emit signalDataRecv(data);
}

void ComDeviceSerial::slotError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }
    qWarning() << (QString("Serial port error: %1").arg(error));
    emit finished();
}
