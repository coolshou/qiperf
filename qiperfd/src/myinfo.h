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
    QString disableInfo();
    QString updateInfo();
//    QHostAddress getIPfromIfname(QString ifname);
    QList<QHostAddress> getIPfromIfname(QString ifname);

    int getEndpointType();
public slots:
    void setIfname(QString mgr_ifname);

signals:

private:
    QString m_ifname;
    int update=0;
};

#endif // MYINFO_H
