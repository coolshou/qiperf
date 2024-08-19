#ifndef ICMPWRAPPER_H
#define ICMPWRAPPER_H

#include <QObject>
#include <QMap>

class IcmpWrapper : public QObject
{
    Q_OBJECT
public:
    explicit IcmpWrapper(QString target, quint64 count=4, bool isV4=true, QObject *parent = nullptr);
    bool pingHost(const QString &hostname, quint64 idx);

public slots:
    void start();
    void stop();

signals:
    void finished();
    void ttl(quint64 idx, int ttl);

private:
    quint64 m_idx;
    QString m_target;
    quint64 m_count; // TODO
    bool m_isV4; // TODO
    int m_interval=1; // TODO
    QMap<quint64, int> m_ttls; // idx, ttl: -1 fail
    // TODO: size
};

#endif // ICMPWRAPPER_H
