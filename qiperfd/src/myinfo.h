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
#if defined(Q_OS_LINUX)
    QString getDriverVersion(const QString &interfaceName, QString &drivername);
#endif
#if defined(Q_OS_WIN32)
    QString getDriverVersion(const QString &interfaceName, QString &drivername);
    QString getDriverVersion(const QString &hardwareID);
    QString getAdapterName(const QString &description);
    void getNetworkAdapterInfo();
#endif
public slots:
    void setIfname(QString mgr_ifname);

signals:

private:
    QString m_ifname;
    int update=0;
//#if defined(Q_OS_WIN32)
    QMap<QString, QString> drivers;
//#endif
};

#endif // MYINFO_H
