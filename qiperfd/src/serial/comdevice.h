#ifndef COMDEVICE_H
#define COMDEVICE_H

#include <QObject>

class ComDevice : public QObject
{
    Q_OBJECT
public:
    explicit ComDevice(QObject *parent = nullptr);
    virtual ~ComDevice();

public slots:
    virtual void init();
    virtual void slotDataSend(const QByteArray& data);

signals:
    void finished();
    void signalDataRecv(const QByteArray& data);

private:
    Q_DISABLE_COPY(ComDevice)


};

#endif // COMDEVICE_H
