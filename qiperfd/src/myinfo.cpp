#include "myinfo.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QDir>
#include <QFile>
#include <QSerialPortInfo>

#if defined(Q_OS_LINUX)
#include <unistd.h> //readlink
//#include <fstream> //file
#include <sys/ioctl.h>
#include <net/if.h>
#endif
#if defined(Q_OS_WIN32)
#include <Windows.h>
#include <iphlpapi.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <SetupAPI.h>
#include <cfgmgr32.h>
#endif

#include "endpoint.h"
#include "endpointtype.h"
#include "endpointact.h"
#include "version.h"
#include "comm.h"

#include <QDebug>


MyInfo::MyInfo(QString mgr_ifname, QObject *parent)
    : QObject{parent}
{
    m_ifname = mgr_ifname;

}

QString MyInfo::collectInfo()
{
    //collect Info for use , in json format string
    /*
 { "ACT":1,
   "Type":,
    "HostName": "Test",
   "OS":"", ""OSVer",
"MB_Vendor":,
"MB_Model":,
"MB_Serial":,
"CPU":,
"MEM":,
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
   "serial":""
   "update":0
 }
*/
#if defined(Q_OS_WIN32)
    getNetworkAdapterInfo();
#endif

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
    QString osver = QSysInfo::kernelVersion();
#if defined(Q_OS_WIN32)
    osver = osver + "." + getWindowsPatchNumber();
#endif
    mainObject.insert("OSVer", osver);
    // motherboard info
    QString vendor="";
    QString model="";
    QString serial="";
    getMotherboardInfo(vendor, model, serial);
    mainObject.insert("MB_Vendor", vendor);
    mainObject.insert("MB_Model", model);
    mainObject.insert("MB_Serial", serial);
    //TODO: CPU/MEM
    QString cpu="";
    QString mem="";
    getCpuMemInfo(cpu, mem);
    mainObject.insert("CPU", cpu);
    mainObject.insert("MEM", mem);

    mainObject.insert("Manager", m_ifname);
    mainObject.insert("update", update);
    mainObject.insert("qiperfd", QString(QIPERFD_VERSION));
    mainObject.insert("buildver", QString(GITBRANCH)+"-"+QString(GITVER));
    //iperf version
    mainObject.insert("iperf2ver", m_iperf2ver);
    mainObject.insert("iperf21ver", m_iperf21ver);
    mainObject.insert("iperf3ver", m_iperf3ver);
    //serial
    mainObject.insert("serial", collectSerial());

    QJsonObject netObject=collectNetInfo();
    mainObject.insert("Net", netObject);

    m_mainObject = mainObject;
    QJsonDocument jsonDoc;
    jsonDoc.setObject(mainObject);
    //conver to QString
//    QString strJson(jsonDoc.toJson(QJsonDocument::Indented));
    QString strJson(jsonDoc.toJson(QJsonDocument::Compact));
    update = 0;
    return strJson;
}

QJsonObject MyInfo::collectNetInfo()
{
    QJsonObject netObjects;
    //獲取所有網路介面的列表
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface niface,list) //遍歷每一個網路介面
    {
        if ((niface.type() == QNetworkInterface::Ethernet) ||
            (niface.type() == QNetworkInterface::Wifi)) {
            QJsonObject ifObject;
            ifObject.insert("HW", niface.hardwareAddress()); //硬體地址
            QString drivername="";
            QString ver =getDriverVersion(niface.name(), drivername);
//            qDebug() << interface.name() << " version: " <<ver << " driver: " << drivername;
            ifObject.insert("driverVersion", ver);             //driver version
            ifObject.insert("driverName", drivername);             //driver name
            QJsonArray addrsObject;
            //獲取IP地址條目列表，每個條目中包含一個IP地址，一個子網掩碼和一個廣播地址
            QList<QNetworkAddressEntry> entryList= niface.addressEntries();
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
            netObjects.insert(niface.humanReadableName(), ifObject);

        }
    }
    return netObjects;
}

QJsonArray MyInfo::collectSerial()
{
    QStringList serials;
    QList<QSerialPortInfo> qs = QSerialPortInfo::availablePorts();
    foreach(auto q, qs){
        if (q.hasProductIdentifier() && q.hasVendorIdentifier()){
            qDebug() << q.portName() << " productIdentifier:" << q.productIdentifier()
                     << " vendorIdentifier:" << q.vendorIdentifier()
                     << " serialNumber:" << q.serialNumber()
                     << " description: " << q.manufacturer();
            serials.append(q.portName());
        }
    }
    qDebug() << "collectSerial:" << serials;

    QJsonArray arr= QJsonArray::fromStringList(serials);
    return arr;
}

QString MyInfo::disableInfo()
{
    QJsonObject mainObject = m_mainObject;
    mainObject.insert("ACT", EndPointAct::Disable);
    QJsonDocument jsonDoc;
    jsonDoc.setObject(mainObject);
    QString strJson(jsonDoc.toJson(QJsonDocument::Compact));
    return strJson;
}

QString MyInfo::updateInfo()
{
    if (QString::compare(m_old_manager_ip, m_new_manager_ip, Qt::CaseInsensitive)==0) {
        qDebug() << "m_old_manager_ip = m_new_manager_ip (" << m_new_manager_ip << ")" ;
        return "";
    }

    QJsonObject mainObject = m_mainObject;
    mainObject.insert("ACT", EndPointAct::Update);
    //from manager IP to new IP
    mainObject.insert("old_manager_ip", m_old_manager_ip);
    mainObject.insert("new_manager_ip", m_new_manager_ip);
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

    foreach (QNetworkInterface niface, interfaces) {
        if (niface.name() == ifname) {
            QList<QNetworkAddressEntry> addresses = niface.addressEntries();
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
        if (QFile::exists("/usr/bin/freebsd-version")) {
            rc = static_cast<int>(EndPointType::FreeBSD);
        }else{
            rc = static_cast<int>(EndPointType::Linux);
        }
        break;
    case 6: // Unknown
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

void MyInfo::getTTL()
{
#if defined(Q_OS_LINUX)
    QString v4buf = readSysFile(IPv4_TTL_PATH);
    QString v6buf = readSysFile(IPv6_TTL_PATH);
    qDebug () << "IPv4 ttl: " << v4buf << " IPv6 ttl:" << v6buf;
#else
    qDebug() << "getTTL: Not support platform: " << QSysInfo::productType();
#endif
}

void MyInfo::setIperfVer(QString v2, QString v21, QString v3)
{
    m_iperf2ver = v2;
    m_iperf21ver = v21;
    m_iperf3ver = v3;
}

QString MyInfo::readSysFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file" << path << ":" << file.errorString();
        return QString();
    }

    QTextStream in(&file);
    //only read one line
    QString content = in.readLine().trimmed();
    // qDebug() << "content.lastIndexOf: " << content.lastIndexOf("\u0000");
    //content.truncate(content.lastIndexOf("\u0000"));// raspiberry 3 fix =>  may cause problem!!
    // qDebug() << path << " : " << content;
    file.close();

    return content;
}

void MyInfo::writeSysFile(const QString &path, QString value)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file" << path << ":" << file.errorString();
        return ;
    }
    QTextStream out(&file);
    out << value;
    out.flush();
    file.close();
}
QString MyInfo::readFileContent(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file" << filePath << ":" << file.errorString();
        return QString();
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    return content;
}
void MyInfo::getMotherboardInfo(QString &vendor,QString &model, QString &serial) {
    // respiberry 3 does not have /sys/class/dmi
    QFile f("/proc/device-tree/name");
    if (f.exists()){
        vendor = readSysFile("/proc/device-tree/name");
    }else{
        vendor = readSysFile("/sys/class/dmi/id/board_vendor");
    }
    f.setFileName("/proc/device-tree/model");
    if (f.exists()){
        model = readSysFile("/proc/device-tree/model");
    }else{
        model = readSysFile("/sys/class/dmi/id/board_name");
    }
    f.setFileName("/proc/device-tree/serial-number");
    if (f.exists()){
        serial = readSysFile("/proc/device-tree/serial-number");
    }else{
        serial = readSysFile("/sys/class/dmi/id/board_serial");
    }

    qInfo() << "Motherboard Vendor:" << vendor;
    qInfo() << "Motherboard Model:" << model;
    qInfo() << "Motherboard Serial Number:" << serial;
}
QString MyInfo::getCPUModel() {
    QString hardware=nullptr;
    QString revision=nullptr;
    QString cpuInfo = readFileContent("/proc/cpuinfo");
    QStringList lines = cpuInfo.split('\n');
    for (const QString &line : lines) {
        if (line.startsWith("model name")) {
            return line.split(':').last().trimmed();
        }
        if (line.startsWith("Hardware")) {
            hardware = line.split(':').last().trimmed();
        }else if (line.startsWith("Revision")) {
            revision = line.split(":").at(1).trimmed();
        }
    }
    if (!hardware.isNull()){
        return hardware + " " + revision;
    }
    return QString("Unknown CPU model");
}

QString MyInfo::getTotalMemory() {
    QString memInfo = readFileContent("/proc/meminfo");
    QStringList lines = memInfo.split('\n');
    for (const QString &line : lines) {
        if (line.startsWith("MemTotal")) {
            return line.split(':').last().trimmed();
        }
    }
    return QString("Unknown Memory");
}
#endif

quint64 MyInfo::getSysBufferSize()
{   //return support Max buffer size in KB

#if defined(Q_OS_LINUX)
    // /proc/sys/net/core/rmem_max
    // /proc/sys/net/core/wmem_max
    // ubuntu 24.04
    // sysctl show net.core.wmem_max  => 212992/1024 => 208K => Max -w 416K
    // -w 4M
    QString rbuf = readSysFile(READ_BUFFER_SIZE_PATH);
    QString wbuf = readSysFile(WRITE_BUFFER_SIZE_PATH);
    qDebug() << "rbuf: " << rbuf << " wbuf: " << wbuf;
    quint64 irbuf = static_cast<quint64>((rbuf.toInt()/static_cast<int>(BUFFER_SIZES::KB))*2);
    quint64 iwbuf = static_cast<quint64>((wbuf.toInt()/static_cast<int>(BUFFER_SIZES::KB))*2);
    if (irbuf > iwbuf) {
        return iwbuf;
    }else{
        return irbuf;
    }
#else
    qDebug() << "getSysBufferSize: Not support platform: " << QSysInfo::productType();
    return 0;
#endif
}

void MyInfo::setSysBufferSize(quint64 buff)
{
#if defined(Q_OS_LINUX)
    // max 416K
    // sudo sysctl net.core.wmem_max=2097152
    // sudo sysctl net.core.rmem_max=2097152
    //# allow TCP with buffers up to 64MB
    //net.core.rmem_max = 67108864
    //net.core.wmem_max = 67108864
    writeSysFile(READ_BUFFER_SIZE_PATH, QString::number(buff/2));
    writeSysFile(WRITE_BUFFER_SIZE_PATH, QString::number(buff/2));
#else
    Q_UNUSED(buff)
    qDebug() << "TODO setSysBufferSize: Not support platform: " << QSysInfo::productType();

#endif
}
void MyInfo::getCpuMemInfo(QString &cpuModel,QString &totalMemory) {
    cpuModel = getCPUModel();
    totalMemory = getTotalMemory();

    qInfo() << "CPU Model:" << cpuModel;
    qInfo() << "Total Memory:" << totalMemory;
}


#if defined(Q_OS_WIN32)
QString MyInfo::getLastErrorAsString() {
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return QString(); // No error message has been recorded
    }

    LPWSTR messageBuffer = nullptr;
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

    QString message = "["+QString::number(errorMessageID)+"]"+ QString::fromWCharArray(messageBuffer, size);

    // Free the buffer allocated by FormatMessage
    LocalFree(messageBuffer);

    return message;
}

QString MyInfo::getDriverVersion(const QString &interfaceName, QString &drivername)
{
    if (drivers.contains(interfaceName)){
        qDebug() << interfaceName << " getDriverVersion: " << drivers[interfaceName][0];
        drivername = drivers[interfaceName][1];
        return drivers[interfaceName][0];

    }
    return QString();
}
QString MyInfo::getDriverVersion(const QString &hardwareID) {
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(NULL, hardwareID.toStdWString().c_str(), NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        qWarning() << "SetupDiGetClassDevs failed: " << hardwareID.toStdWString() << "\n" << getLastErrorAsString();
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
                }else{
                    qDebug() << "Fail: RegQueryValueEx: " << hKey;
                }
                RegCloseKey(hKey);
            }else{
                qDebug() << "Fail: RegOpenKeyEx: " << driverVersionKey;
            }
        }else{
            qDebug() << "Fail: SetupDiGetDeviceRegistryProperty" ;
        }
    }else{
        qDebug() << "Fail: SetupDiEnumDeviceInfo" ;
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
//    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
    hres = pLoc->ConnectServer(SysAllocString(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
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
    //hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_NetworkAdapter"), WBEM_FLAG_FORWARD_ONLY |
    hres = pSvc->ExecQuery(SysAllocString(L"WQL"), SysAllocString(L"SELECT * FROM Win32_NetworkAdapter"), WBEM_FLAG_FORWARD_ONLY |
                           WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
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
            // qDebug() << "Adapter Name:" << pAdapter->AdapterName;
            // qDebug() << "Description:" << pAdapter->Description;
            QString hardwareID = getAdapterName(QString::fromLocal8Bit(pAdapter->Description));
            if (!hardwareID.isEmpty()) {
                QString driverVersion = getDriverVersion(hardwareID);
                if (!driverVersion.isEmpty()) {
                    drivers[pAdapter->AdapterName].append(driverVersion);
                    drivers[pAdapter->AdapterName].append(QString(pAdapter->Description).trimmed());
                    qInfo() << "\"" << drivers[pAdapter->AdapterName][1] << "\" Driver Version: " << driverVersion;
                } else {
                    // qDebug() << "Driver version not found for adapter\"" << pAdapter->Description << "\"";
                }
            } else {
                // qDebug() << "Hardware ID not found for adapter" << pAdapter->Description;
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
QString MyInfo::getWindowsPatchNumber(){
    HKEY hKey;
    const wchar_t* subKey = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return "Can not open Registry";
    }

    DWORD patchNumber = 0;
    DWORD valueSize = sizeof(patchNumber);
    DWORD type = 0;
    if (RegQueryValueExW(hKey, L"UBR", nullptr, &type, reinterpret_cast<LPBYTE>(&patchNumber), &valueSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return "Can not read patchNumber (UBR)";
    }

    RegCloseKey(hKey);
    qDebug() << "patchNumber:" << patchNumber;
    return QString::number(patchNumber);
}
void MyInfo::getMotherboardInfo(QString &vendor,QString &model, QString &serial) {
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        qWarning() << "Failed to initialize COM library:" << getLastErrorAsString();
        return;
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hres)) {
        qWarning() << "Failed to initialize security:" << getLastErrorAsString();
        CoUninitialize();
        return;
    }

    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
    if (FAILED(hres)) {
        qWarning() << "Failed to create IWbemLocator object:" << getLastErrorAsString();
        CoUninitialize();
        return;
    }

    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(SysAllocString(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
    if (FAILED(hres)) {
        qWarning() << "Could not connect to WMI:" << getLastErrorAsString();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    if (FAILED(hres)) {
        qWarning() << "Could not set proxy blanket:" << getLastErrorAsString();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(SysAllocString(L"WQL"), SysAllocString(L"SELECT * FROM Win32_BaseBoard"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    if (FAILED(hres)) {
        qWarning() << "Query for Win32_BaseBoard failed:" << getLastErrorAsString();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IWbemClassObject *pClsObj = NULL;
    ULONG uReturn = 0;
    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClsObj, &uReturn);
        if (0 == uReturn) break;

        VARIANT vtProp;

        // Get the motherboard vendor
        hr = pClsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
            vendor = QString::fromWCharArray(vtProp.bstrVal);
            qInfo() << "Motherboard Manufacturer:" << vendor;
        }
        VariantClear(&vtProp);

        // Get the motherboard model
        hr = pClsObj->Get(L"Product", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
            model = QString::fromWCharArray(vtProp.bstrVal);
            qInfo() << "Motherboard Model:" << model;
        }
        VariantClear(&vtProp);

        // Get the motherboard serial number
        hr = pClsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
            serial = QString::fromWCharArray(vtProp.bstrVal);
            qInfo() << "Motherboard Serial Number:" << serial;
        }
        VariantClear(&vtProp);

        pClsObj->Release();
    }

    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
}

QString MyInfo::getWMIProperty(IWbemClassObject* pClsObj, const BSTR property) {
    VARIANT vtProp;
    QString result;
    HRESULT hr = pClsObj->Get(property, 0, &vtProp, 0, 0);
    if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
        result = QString::fromWCharArray(vtProp.bstrVal);
    }
    VariantClear(&vtProp);
    return result;
}

QString MyInfo::getCPUModel() {
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        qWarning() << "Failed to initialize COM library:" << getLastErrorAsString();
        return QString();
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hres)) {
        qWarning() << "Failed to initialize security:" << getLastErrorAsString();
        CoUninitialize();
        return QString();
    }

    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
    if (FAILED(hres)) {
        qWarning() << "Failed to create IWbemLocator object:" << getLastErrorAsString();
        CoUninitialize();
        return QString();
    }

    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(SysAllocString(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
    if (FAILED(hres)) {
        qWarning() << "Could not connect to WMI:" << getLastErrorAsString();
        pLoc->Release();
        CoUninitialize();
        return QString();
    }

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    if (FAILED(hres)) {
        qWarning() << "Could not set proxy blanket:" << getLastErrorAsString();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return QString();
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(SysAllocString(L"WQL"), SysAllocString(L"SELECT * FROM Win32_Processor"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    if (FAILED(hres)) {
        qWarning() << "Query for Win32_Processor failed:" << getLastErrorAsString();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return QString();
    }

    IWbemClassObject *pClsObj = NULL;
    ULONG uReturn = 0;
    QString cpuModel;
    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClsObj, &uReturn);
        if (0 == uReturn) {
            break;
        }

        cpuModel = getWMIProperty(pClsObj, SysAllocString(L"Name"));
        pClsObj->Release();
    }

    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return cpuModel;
}

QString MyInfo::getTotalMemory() {
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        qWarning() << "Failed to initialize COM library:" << getLastErrorAsString();
        return QString();
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hres)) {
        qWarning() << "Failed to initialize security:" << getLastErrorAsString();
        CoUninitialize();
        return QString();
    }

    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);
    if (FAILED(hres)) {
        qWarning() << "Failed to create IWbemLocator object:" << getLastErrorAsString();
        CoUninitialize();
        return QString();
    }

    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(SysAllocString(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
    if (FAILED(hres)) {
        qWarning() << "Could not connect to WMI:" << getLastErrorAsString();
        pLoc->Release();
        CoUninitialize();
        return QString();
    }

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    if (FAILED(hres)) {
        qWarning() << "Could not set proxy blanket:" << getLastErrorAsString();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return QString();
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(SysAllocString(L"WQL"), SysAllocString(L"SELECT * FROM Win32_OperatingSystem"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    if (FAILED(hres)) {
        qWarning() << "Query for Win32_OperatingSystem failed:" << getLastErrorAsString();
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return QString();
    }

    IWbemClassObject *pClsObj = NULL;
    ULONG uReturn = 0;
    QString totalMemory;
    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClsObj, &uReturn);
        if (0 == uReturn) break;

        VARIANT vtProp;
        hr = pClsObj->Get(L"TotalVisibleMemorySize", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR) {
            // Convert KB to GB
            double totalMemoryKB = _wtof(vtProp.bstrVal);
            totalMemory = QString::number(totalMemoryKB / (1024 * 1024), 'f', 2) + " GB";
        }
        VariantClear(&vtProp);

        pClsObj->Release();
    }

    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    return totalMemory;
}

#endif
void MyInfo::setIfname(QString mgr_ifname)
{
    QList<QHostAddress> old_addrs, new_addrs;

    //org manager ip
    old_addrs = getIPfromIfname(m_ifname);
    if (old_addrs.length()>0){
        m_old_manager_ip = old_addrs[0].toString();
    }
    //new manager ip
    m_ifname = mgr_ifname;
    new_addrs = getIPfromIfname(m_ifname);
    if (new_addrs.length()>0){
        m_new_manager_ip = new_addrs[0].toString();
    }
    update = 1;
}
