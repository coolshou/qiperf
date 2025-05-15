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
    explicit SerialTask(QString midx, const QString& serialPortName, const QString& serialBaudRate,
                        const QString& localIp, const QString& localPort,
                        ComDeviceTcp::Mode mode,
                        QSerialPort::DataBits serialDataBits = QSerialPort::Data8,
                        QSerialPort::Parity serialParity = QSerialPort::NoParity,
                        QSerialPort::StopBits serialStopBits = QSerialPort::OneStop,
                        QSerialPort::FlowControl serialFlowControl = QSerialPort::NoFlowControl,
                        bool localInput=false, bool localOutput=false, QObject *parent = nullptr);
    ~SerialTask();
    quint16 getLocalPort();
    QString getIdx();
    bool isRunning();
    QString getLastError();
    void setConfig(QString localPort, QString serialBaudRate,
                   QSerialPort::DataBits serialDataBits = QSerialPort::Data8,
                   QSerialPort::Parity serialParity = QSerialPort::NoParity,
                   QSerialPort::StopBits serialStopBits = QSerialPort::OneStop,
                   QSerialPort::FlowControl serialFlowControl = QSerialPort::NoFlowControl);
public slots:
    void init();
    void close();
    void onStarted(QString idx, quint16 port);

signals:
    void finished(QString serialPortName);
    void started(QString idx, quint16 port);// notice run on which port

private:
    Q_DISABLE_COPY(SerialTask)

private slots:
    void slotFinished();


private:
    QString m_idx;
    QString _serialPortName;
    QString _serialBaudRate;
    QString _localIp;
    QString _localPort;
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
    QString _lasterror;
};

#endif // SERIALTASK_H
