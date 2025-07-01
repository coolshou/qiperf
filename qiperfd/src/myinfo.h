#ifndef MYINFO_H
#define MYINFO_H

#include <QJsonObject>
#include <QObject>
#include <QHostAddress>

#if defined(Q_OS_WIN32)
#include "wbemcli.h"
#include "winreg.h"
#endif

class MyInfo : public QObject
{
    Q_OBJECT
public:
    explicit MyInfo(QString mgr_ifname, QObject *parent = nullptr);

    QString collectInfo();
    QJsonObject collectNetInfo();
    QJsonArray collectSerial();
    QString disableInfo();
    QString updateInfo();
//    QHostAddress getIPfromIfname(QString ifname);
    QList<QHostAddress> getIPfromIfname(QString ifname);

    int getEndpointType();
    void setIperfVer(QString v2,QString v21, QString v22, QString v3);
    QJsonObject getIperfVer();
    void getCpuMemInfo(QString &cpuModel, QString &totalMemory, int& cpucorenum);
    void getMotherboardInfo(QString &vendor, QString &model, QString &serial);
    QString getDriverVersion(const QString &interfaceName, QString &drivername);
    quint64 getSysBufferSize();
    void setSysBufferSize(quint64 buff);
    void getTTL();

#if defined(Q_OS_WIN32)
    QString getHResultErrorString(HRESULT hr);
    QString getLastErrorAsString();
    QString getDriverVersion(const QString &hardwareID);
    QString getAdapterName(const QString &description);
    void getNetworkAdapterInfo();
    QString getWindowsPatchNumber();
#endif

public slots:
    void setIfname(QString mgr_ifname);
signals:

private:
    QString getTotalMemory();
    QString getCPUModel(int& corenum);
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
    QString m_old_manager_ip;
    QString m_new_manager_ip;
    QJsonObject m_mainObject;
    QString m_iperf20ver;
    QString m_iperf21ver;
    QString m_iperf22ver;
    QString m_iperf3ver;
};

#endif // MYINFO_H
