#ifndef MYINFO_H
#define MYINFO_H

#include <QJsonObject>
#include <QObject>
#include <QHostAddress>

class MyInfo : public QObject
{
    Q_OBJECT
public:
    explicit MyInfo(QString mgr_ifname, QObject *parent = nullptr);

    QString collectInfo();
    QJsonObject collectNetInfo();
//    QHostAddress getIPfromIfname(QString ifname);
    QList<QHostAddress> getIPfromIfname(QString ifname);
signals:

private:
    QString m_ifname;
};

#endif // MYINFO_H
