#ifndef ICMPPING_H
#define ICMPPING_H

#include <QObject>
#include <oping.h>

class IcmpPing : public QObject
{
    Q_OBJECT
public:
    explicit IcmpPing(QObject *parent = nullptr);
    bool pingHost(const QString &hostname, int count = 1);

signals:
    void pingSuccess(int responseTime);
    void pingFailed();
};

#endif // ICMPPING_H
