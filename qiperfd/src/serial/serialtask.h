#ifndef SERIALTASK_H
#define SERIALTASK_H

#include <QObject>
#include <QSerialPort>

#include "comdeviceserial.h"
#include "comdevicetcp.h"

class SerialTask: public QObject
{
    Q_OBJECT
public:
    explicit SerialTask(const QString& serialPortName, const QString& serialBaudRate,
                        const QString& localIp, const QString& localPort,
                        ComDeviceTcp::Mode mode,
                        QSerialPort::DataBits serialDataBits = QSerialPort::Data8,
                        QSerialPort::Parity serialParity = QSerialPort::NoParity,
                        QSerialPort::StopBits serialStopBits = QSerialPort::OneStop,
                        QSerialPort::FlowControl serialFlowControl = QSerialPort::NoFlowControl,
                        bool localInput=false, bool localOutput=false, QObject *parent = nullptr);
    ~SerialTask();


public slots:
    void init();
    void close();

signals:
    void finished();

private:
    Q_DISABLE_COPY(SerialTask)

private slots:
    void slotFinished();

private:
    const QString _serialPortName;
    const QString _serialBaudRate;
    const QString _localIp;
    const QString _localPort;
    const ComDeviceTcp::Mode _mode;
    QSerialPort::DataBits _serialDataBits;
    QSerialPort::Parity _serialParity;
    QSerialPort::StopBits _serialStopBits;
    QSerialPort::FlowControl _serialFlowControl;
    const bool _localInput;
    const bool _localOutput;

    ComDeviceSerial *_comDeviceSerial;
    ComDeviceTcp *_comDeviceTcp;
    // ComDevice* _comDeviceScreen;
};

#endif // SERIALTASK_H
