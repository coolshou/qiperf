#ifndef COMDEVICESERIAL_H
#define COMDEVICESERIAL_H

#include <QObject>
#include <QSerialPort>

#include "comdevice.h"

class ComDeviceSerial : public ComDevice
{
    Q_OBJECT
public:
    explicit ComDeviceSerial(const QString& serialPortName, const QString& serialBaudRate,
                             QObject *parent = nullptr,
                             QSerialPort::DataBits serialDataBits = QSerialPort::Data8,
                             QSerialPort::Parity serialParity = QSerialPort::NoParity,
                             QSerialPort::StopBits serialStopBits = QSerialPort::OneStop,
                             QSerialPort::FlowControl serialFlowControl = QSerialPort::NoFlowControl
                             );
    virtual ~ComDeviceSerial();

public slots:
    virtual void init();
    virtual void slotDataSend(const QByteArray& data);
    void close();
    void slotReadyRead();
    void slotError(QSerialPort::SerialPortError error);

private:
    Q_DISABLE_COPY(ComDeviceSerial)

private:
    const QString _serialPortName;
    const QString _serialBaudRate;
    QSerialPort::DataBits _serialDataBits;
    QSerialPort::Parity _serialParity;
    QSerialPort::StopBits _serialStopBits;
    QSerialPort::FlowControl _serialFlowControl;
    QSerialPort* _serialPort;
};

#endif // COMDEVICESERIAL_H
