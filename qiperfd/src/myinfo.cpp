#include "myinfo.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QDir>

#if defined(Q_OS_LINUX)
#include <unistd.h> //readlink
//#include <fstream> //file
#include <sys/ioctl.h>
#include <net/if.h>
#endif
#if defined(Q_OS_WIN32)
#include <iphlpapi.h>
#endif

#include "endpoint.h"
#include "endpointtype.h"
#include "endpointact.h"
#include "version.h"

#include <QDebug>


MyInfo::MyInfo(QString mgr_ifname, QObject *parent)
    : QObject{parent}
{
    m_ifname = mgr_ifname;
#if defined(Q_OS_WIN32)
    getNetworkAdapterInfo();
#endif
}

QString MyInfo::collectInfo()
{
    //collect Info for use , in json format string
    /*
 { "ACT":,
   "Type":,
    "HostName": "Test",
   "OS":"", ""OSVer",
   "Manager": "ifname1",
   "Net": {
       "ifname1": {
            "HW":"",
            "Addrs":[["ip", "Mask", "BCast"],["ip2", "Mask2", "BCast2"]],
       "ifname2": {
            "HW":"",
            "Addrs":[["ip", "Mask", "BCast"],["ip2", "Mask2", "BCast2"]]
   },
   "qiperfd":"0.2.11306.21"
   "update":0
 }
*/

    QJsonObject mainObject;
    mainObject.insert("ACT", EndPointAct::Add);
    mainObject.insert("Type", getEndpointType());
    mainObject.insert("HostName", QHostInfo::localHostName());
    if (EndPointType::Windows==getEndpointType()){
        //Windows 11
        QString OS = "Windows 10";
        QString OSVer = QSysInfo::kernelVersion();
        QStringList ds = OSVer.split(".");
        if (ds[0].toInt()==10){
            //Windows 10/11
            if (ds[2].toInt()>19045){
                OS = "Windows 11";
            }
        }else{
            OS = QSysInfo::prettyProductName();
        }
        mainObject.insert("OS", OS);
    }else{
        mainObject.insert("OS", QSysInfo::prettyProductName());
    }
    mainObject.insert("OSVer", QSysInfo::kernelVersion());
    mainObject.insert("Manager", m_ifname);
    mainObject.insert("update", update);
    mainObject.insert("qiperfd", QString(QIPERFD_VERSION));

    QJsonObject netObject=collectNetInfo();

    mainObject.insert("Net", netObject);
    QJsonDocument jsonDoc;
    jsonDoc.setObject(mainObject);
    //conver to QString
//    QString strJson(jsonDoc.toJson(QJsonDocument::Indented));
    QString strJson(jsonDoc.toJson(QJsonDocument::Compact));
//    qDebug().noquote() << "JSON:" << strJson << Qt::endl;
    update = 0;
    return strJson;
}

QJsonObject MyInfo::collectNetInfo()
{
    QJsonObject netObjects;
    //獲取所有網路介面的列表
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface interface,list) //遍歷每一個網路介面
    {
        if ((interface.type() == QNetworkInterface::Ethernet) ||
            (interface.type() == QNetworkInterface::Wifi)) {
            QJsonObject ifObject;
            ifObject.insert("HW", interface.hardwareAddress()); //硬體地址
            QString drivername="";
            QString ver =getDriverVersion(interface.name(), drivername);
//            qDebug() << interface.name() << " version: " <<ver << " driver: " << drivername;
            ifObject.insert("driverVersion", ver);             //driver version
            ifObject.insert("driverName", drivername);             //driver name
            QJsonArray addrsObject;
            //獲取IP地址條目列表，每個條目中包含一個IP地址，一個子網掩碼和一個廣播地址
            QList<QNetworkAddressEntry> entryList= interface.addressEntries();
            foreach(QNetworkAddressEntry entry,entryList)//遍歷每個IP地址條目
            {
                QJsonArray addrObject;
                addrObject.push_back(entry.ip().toString());//IP地址
                addrObject.push_back(entry.netmask().toString());//子網掩碼
                addrObject.push_back(entry.broadcast().toString());//廣播地址
                addrsObject.push_back(addrObject);
            }
            ifObject.insert("address", addrsObject);
//            netObjects.insert(interface.name(), ifObject);
            netObjects.insert(interface.humanReadableName(), ifObject);

        }
    }
    return netObjects;
}

QString MyInfo::disableInfo()
{
    QJsonObject mainObject;
    mainObject.insert("ACT", EndPointAct::Disable);
    QJsonDocument jsonDoc;
    jsonDoc.setObject(mainObject);
    QString strJson(jsonDoc.toJson(QJsonDocument::Compact));
    return strJson;
}

QString MyInfo::updateInfo()
{
    QJsonObject mainObject;
    mainObject.insert("ACT", EndPointAct::Update);
    //TODO: other update info
    QJsonDocument jsonDoc;
    jsonDoc.setObject(mainObject);
    QString strJson(jsonDoc.toJson(QJsonDocument::Compact));
    return strJson;
}

QList<QHostAddress> MyInfo::getIPfromIfname(QString ifname)
{
    QList<QHostAddress> addrs;
    QHostAddress ipAddress=QHostAddress("0.0.0.0");
    QHostAddress bAddress=QHostAddress("255.255.255.255");

    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    foreach (QNetworkInterface interface, interfaces) {
        if (interface.name() == ifname) {
            QList<QNetworkAddressEntry> addresses = interface.addressEntries();
            foreach (QNetworkAddressEntry address, addresses) {
                if (address.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    ipAddress = address.ip();
                    addrs.append(ipAddress);
                    bAddress = address.broadcast();
                    addrs.append(bAddress);
                    break;
                }
            }
            break;
        }
    }
//    return ipAddress;
    return addrs;
}

int MyInfo::getEndpointType()
{
//    EndPoint::Type rc = EndPoint::Unknown;
    int rc = static_cast<int>(EndPointType::Unknown);
    QStringList myOptions;
    myOptions << "windows" << "android" << "macos" << "osx" << "ios" << "debian" << "unknown" ;

    switch(myOptions.indexOf(QSysInfo::productType())){
    case 0: //windows
        rc = static_cast<int>(EndPointType::Windows);
        break;
    case 1: //android
        rc = static_cast<int>(EndPointType::Android);
        break;

    case 2: //macos
    case 3: //osx
        rc = static_cast<int>(EndPointType::MacOS);
        break;
    case 4: //ios
        rc = static_cast<int>(EndPointType::iOS);
        break;
    case 5: //debian/FreeBSD
        rc = static_cast<int>(EndPointType::FreeBSD);
        break;
    case 6:
        rc = static_cast<int>(EndPointType::Unknown);
        break;
    default:// Linux
        rc = static_cast<int>(EndPointType::Linux);
        break;
    }
    return rc;

//    QString tpy =QSysInfo::productType();
//    if (QString::compare(tpy, "android", Qt::CaseInsensitive)==0){
//    }else if (QString::compare(tpy, "macos", Qt::CaseInsensitive)==0){
    //    }
}

#if defined(Q_OS_LINUX)
QString MyInfo::getDriverVersion(const QString &interfaceName, QString &drivername)
{
    //get Linux interfaceName driver version;
    QString driverPath = QString("/sys/class/net/%1/device/driver/module").arg(interfaceName);
    QFile driverLink(driverPath);
    drivername = QDir(driverLink.symLinkTarget()).dirName();

    QString verPath = driverPath + "/version";
//    qDebug() << "verPath:" << verPath;
    QFile file(verPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString version = in.readLine();
        file.close();
        return version;
    }else{
        QString srcverPath = driverPath + "/srcversion";
//        qDebug() << "srcverPath:" << srcverPath;
        QFile file(srcverPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString version = in.readLine();
            file.close();
            return version;
        }
    }
    return QString();
}
#endif
#if defined(Q_OS_WIN32)
QString MyInfo::getDriverVersion(const QString &interfaceName, QString &drivername)
{
    if (drivers->contains(interfaceName)){
        qDebug() << interfaceName << " getDriverVersion: " << drivers.value(interfaceName);
    }
    return QString();
}
QString MyInfo::getDriverVersion(const QString &hardwareID) {
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(NULL, hardwareID.toStdWString().c_str(), NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        qWarning() << "SetupDiGetClassDevs failed";
        return QString();
    }

    SP_DEVINFO_DATA deviceInfoData = {};
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    if (SetupDiEnumDeviceInfo(deviceInfoSet, 0, &deviceInfoData)) {
        DWORD dataType, actualSize = 0;
        BYTE data[256] = {0};

        if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_DRIVER, &dataType, data, sizeof(data), &actualSize)) {
            QString driverKey = QString::fromWCharArray((wchar_t*)data);

            HKEY hKey;
            QString driverVersionKey = "SYSTEM\\CurrentControlSet\\Control\\Class\\" + driverKey;
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, driverVersionKey.toStdWString().c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                DWORD size = 256;
                wchar_t version[256];
                if (RegQueryValueEx(hKey, L"DriverVersion", NULL, NULL, (LPBYTE)version, &size) == ERROR_SUCCESS) {
                    RegCloseKey(hKey);
                    SetupDiDestroyDeviceInfoList(deviceInfoSet);
                    return QString::fromWCharArray(version);
                }
                RegCloseKey(hKey);
            }
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return QString();
}
QString MyInfo::getAdapterName(const QString &description) {
    // WMI initialization
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        qWarning() << "Failed to initialize COM library";
        return QString();
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hres)) {
        qWarning() << "Failed to initialize security";
        CoUninitialize();
        return QString();
    }

    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
    if (FAILED(hres)) {
        qWarning() << "Failed to create IWbemLocator object";
        CoUninitialize();
        return QString();
    }

    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
    if (FAILED(hres)) {
        qWarning() << "Could not connect to WMI";
        pLoc->Release();
        CoUninitialize();
        return QString();
    }

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    if (FAILED(hres)) {
        qWarning() << "Could not set proxy blanket";
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return QString();
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_NetworkAdapter"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    if (FAILED(hres)) {
        qWarning() << "Query for network adapters failed";
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return QString();
    }

    IWbemClassObject *pClsObj = NULL;
    ULONG uReturn = 0;
    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClsObj, &uReturn);
        if (0 == uReturn) break;

        VARIANT vtProp;
        hr = pClsObj->Get(L"Description", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR && QString::fromWCharArray(vtProp.bstrVal) == description) {
            hr = pClsObj->Get(L"PNPDeviceID", 0, &vtProp, 0, 0);
            if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
                QString hardwareID = QString::fromWCharArray(vtProp.bstrVal);
                VariantClear(&vtProp);
                pClsObj->Release();
                pEnumerator->Release();
                pSvc->Release();
                pLoc->Release();
                CoUninitialize();
                return hardwareID;
            }
        }
        VariantClear(&vtProp);
        pClsObj->Release();
    }

    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    return QString();
}
void MyInfo::getNetworkAdapterInfo() {
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter) {
            drivers[pAdapter->AdapterName] = pAdapter->DriverVersion;
            qDebug() << "Adapter Name:" << pAdapter->AdapterName;
            qDebug() << "Description:" << pAdapter->Description;
            //qDebug() << "Driver Version:" << pAdapter->DriverVersion;
            QString hardwareID = getAdapterName(QString::fromLocal8Bit(pAdapter->Description));
            if (!hardwareID.isEmpty()) {
                QString driverVersion = getDriverVersion(hardwareID);
                if (!driverVersion.isEmpty()) {
                    qDebug() << "Driver Version:" << driverVersion;
                } else {
                    qDebug() << "Driver version not found for adapter" << pAdapter->Description;
                }
            } else {
                qDebug() << "Hardware ID not found for adapter" << pAdapter->Description;
            }
            pAdapter = pAdapter->Next;
        }
    } else {
        qWarning() << "GetAdaptersInfo failed";
    }

    if (pAdapterInfo) {
        free(pAdapterInfo);
    }
}
#endif
void MyInfo::setIfname(QString mgr_ifname)
{
    m_ifname = mgr_ifname;
    update = 1;
}
