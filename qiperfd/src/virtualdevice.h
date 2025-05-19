#ifndef VIRTUALDEVICE_H
#define VIRTUALDEVICE_H

#include <QObject>

class VirtualDevice : public QObject
{
    Q_OBJECT
public:
    explicit VirtualDevice(QObject *parent = nullptr);
    virtual ~VirtualDevice();

public slots:
    virtual void init();
    virtual void slotDataSend(const QByteArray& data);

signals:
    void finished();
    void signalDataRecv(const QByteArray& data);

private:
    Q_DISABLE_COPY(VirtualDevice)

};

#endif // VIRTUALDEVICE_H
