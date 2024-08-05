#ifndef MYINFO_H
#define MYINFO_H

#include <QJsonObject>
#include <QObject>
#include <QHostAddress>

#if defined(Q_OS_WIN32)
#include "wbemcli.h"
#endif

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
    void getCpuMemInfo(QString &cpuModel, QString &totalMemory);
    void getMotherboardInfo(QString &vendor, QString &model, QString &serial);
    QString getDriverVersion(const QString &interfaceName, QString &drivername);
    quint64 getSysBufferSize();
    void setSysBufferSize(quint64 buff);
    void getTTL();

#if defined(Q_OS_LINUX)

#endif
#if defined(Q_OS_WIN32)
    QString getLastErrorAsString();
    QString getDriverVersion(const QString &hardwareID);
    QString getAdapterName(const QString &description);
    void getNetworkAdapterInfo();
#endif

public slots:
    void setIfname(QString mgr_ifname);
signals:

private:
    QString getTotalMemory();
    QString getCPUModel();
#if defined(Q_OS_LINUX)
    QString readSysFile(const QString &path);
    void writeSysFile(const QString &path, QString value);
    QString readFileContent(const QString &filePath);
#endif
#if defined(Q_OS_WIN32)
    QString getWMIProperty(IWbemClassObject* pClsObj, const BSTR property);
#endif
    QString m_ifname;
    int update=0;
#if defined(Q_OS_WIN32)
    QMap<QString, QStringList> drivers;
#endif


};

#endif // MYINFO_H
