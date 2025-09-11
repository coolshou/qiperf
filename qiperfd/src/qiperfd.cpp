#include <QCoreApplication>
#include <QStandardPaths>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QNetworkInterface>
#include <QDir>
#include <QEventLoop>
#include <QProcess>
#include <QDateTime>
#include <QSysInfo>
#include <QMetaEnum>
#include <QMutexLocker>

#include "qiperfd.h"
#include "../src/comm.h"
#include "../src/versions.h"

#include "../JIO/jiocmd.h"

#if defined(Q_OS_WIN32)
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#endif
#include <QDebug>

QIperfd::QIperfd(PipeServer *pserver, QObject *parent)
    : QObject{parent}, m_pserver(pserver), m_fileclient(nullptr)
{
    m_debuglv = 3;
    connect(this, &QIperfd::setDebugLv, this, &QIperfd::onSetDebugLv);
    // pserver : interact with systemtray GUI (qiperftray)
    // bReportTPData = false;
    m_ntpserver = nullptr;
    onLog(QString(QIPERFD_NAME) + ":" + QIPERFD_VERSION);
    tmppath = QStandardPaths::writableLocation(QStandardPaths::TempLocation)+
            QDir::separator()+QIPERF_NAME+QDir::separator();
    tmpfilepath = tmppath + "data";
    QDir d(tmpfilepath);
    if (!d.exists()){
        if(!d.mkpath(tmpfilepath)){
            onLog("ERROR: mkdir "+ tmpfilepath+ " Fail");
        }
    }
    // onLog("tmpfilepath: "+ tmpfilepath);
    // TODO: setting
    cfg = new QSettings(QSettings::IniFormat, QSettings::SystemScope,
                              QIPERF_ORG, QIPERFD_NAME);
    onLog("cfg filename:" + cfg->fileName());
    //SystemScope: /etc/xdg/xdg-lxqt/alphanetworks/qiperfd.conf
        //sudo =>      /etc/xdg/alphanetworks/qiperfd.conf
        //windows: C:\ProgramData\alphanetworks\qiperfd.conf

    //UserScope: /home/jimmy/.config/alphanetworks/qiperfd.conf
        //sudo =>       /root/.config/alphanetworks/qiperfd.conf

    QFileInfo fi(cfg->fileName());
    if (!QDir(fi.absolutePath()).exists()){
        QDir().mkdir(fi.absolutePath());
    }
    apppath = qApp->applicationDirPath(); // app run time path:/home/coolshou/sdb/download/work/qiperf/Debug
    m_nssm = QDir::toNativeSeparators(apppath + QDir::separator() +"nssm.exe");
    loadcfg(apppath);
    //
    initIperf(apppath);
    getIperfVer(m_iperfexe20, 2.0);
    getIperfVer(m_iperfexe21, 2.1);
    getIperfVer(m_iperfexe22, 2.2);
    getIperfVer(m_iperfexe3, 3.0);

    m_iperfwrapper = new IperfWrapper();
//    m_myinfo = new MyInfo(getManagerInterface());
    m_myinfo = new MyInfo(mgr_ifname);
    m_myinfo->setIperfVer(m_iperfexe20ver, m_iperfexe21ver, m_iperfexe22ver, m_iperfexe3ver);
    connect(this, &QIperfd::setMgrIfname, m_myinfo, &MyInfo::setIfname);
    QString info = m_myinfo->collectInfo();
    quint64 buffsize = m_myinfo->getSysBufferSize();
    if (buffsize>0){
        debug("Max socket buffer sizes: " + QString::number(buffsize) + " K", 4);
        debug("TODO: set Max socket buffer sizes to ?", 4);
    }
    // m_myinfo->setSysBufferSize(4*static_cast<uint>(BUFFER_SIZES::MB));
    // notice qiperfc info
    m_udpsrv = new UdpSrv(QIPERFD_BPORT, getManagerInterface(), m_myinfo);
    connect(this, &QIperfd::setMgrIfname, m_udpsrv, &UdpSrv::setIfname);
    connect(this, &QIperfd::setDebugLv, m_udpsrv, &UdpSrv::onSetDebugLv);
    m_udpsrv->setSendMsg(info); // broadcast

#if (TEST_WS==1)
    // websocket server : receive/handle cmd from qiperfc
    m_wsserver = new WSServer(QIPERFD_WSPORT, getManagerInterface(), m_myinfo); // websocket listen
    connect(m_wsserver, &WSServer::actMessage, this ,&QIperfd::onWSactMessage);
    connect(m_wsserver, &WSServer::newClient, this ,&QIperfd::onNewClient);
    connect(this, &QIperfd::iperfStarted, m_wsserver, &WSServer::sendTextResult);
    connect(this, &QIperfd::setMgrIfname, m_wsserver, &WSServer::setIfname);
#endif

    startNtpServer();
    m_ntpsync = new NtpSync(this);
    connect(m_ntpsync, &NtpSync::timesynced, this, &QIperfd::onTimeSynced);
    // m_ntpsync->sync("192.168.70.147");

    // system service manager
    qiperfdlog = tmppath+QIPERFD_NAME+".log";
    onLog("FileWatcher: " + QDir::toNativeSeparators(qiperfdlog));
    m_filewatcher = new FileWatcher(qiperfdlog);
    connect(m_filewatcher, &FileWatcher::onNewLine, this, &QIperfd::onNewLine);

    initJIOOpenWRT();

    informMessage(INFO_QIPERFD_STARTED, true);
    checkFirewallStatus();
}

QIperfd::~QIperfd()
{
    infoQIperfdStopped();
    informMessage(INFO_QIPERFD_STOPED, true);
    savecfg();
}


void QIperfd::onLog(QString text)
{
    //qApp->applicationPid() is app's PID
    qInfo() << "(" << qApp->applicationPid() <<")" << text;
}

void QIperfd::loadcfg(QString apppath)
{
    cfg->beginGroup("main");
    cfg->setValue("Path", apppath);
    cfg->endGroup();
    cfg->sync();

    QStringList nls = listInterfaces();
    onLog("net interface list:" + nls.join(" ") );
    // load config setting
    cfg->beginGroup("manager");
    QString default_ifname = "";
    if (!cfg->childKeys().contains("ifname")){
        default_ifname = nls[0];
    }
    mgr_ifname = cfg->value("ifname", default_ifname).toString();
    mgr_port = cfg->value("port", QIPERFD_PORT).toInt();
    // onLog("mgr_ifname: " + mgr_ifname + ", mgr_port: " + QString::number(mgr_port));

    bNtpserver = cfg->value("EnableNtpServer", false).toBool();

    cfg->endGroup();
    cfg->beginGroup("iperf");
    bUseSysIperf = cfg->value("UseSysIperf", false).toBool();
    bIsJIOOpenWRT = cfg->value("JIOOpenWRT", false).toBool();
    bIsAM7 = cfg->value("IsAM7", false).toBool();
    cfg->endGroup();
}

void QIperfd::savecfg()
{
    cfg->beginGroup("manager");
    cfg->setValue("ifname", mgr_ifname);
    cfg->setValue("port", mgr_port);
    cfg->setValue("EnableNtpServer", bNtpserver);
    cfg->endGroup();
    cfg->beginGroup("iperf");
    cfg->setValue("UseSysIperf", bUseSysIperf);
    cfg->endGroup();
    cfg->sync();
}

QStringList QIperfd::listInterfaces()
{
    QStringList nslist;
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface iface, list) // 遍歷每一個網路介面
    {
        if ((iface.type() == QNetworkInterface::Ethernet) ||
            (iface.type() == QNetworkInterface::Wifi))
        {
//            nslist << iface.name();
            nslist << iface.humanReadableName(); //for windows
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return nslist;
}

QString QIperfd::getInterfaceAddr(QString ifname)
{
    QString tmp = "";
    // get first addr of an interface
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface iface, list) // 遍歷每一個網路介面
    {
        if (ifname.compare(iface.name()) == 0)
        {
            if ((iface.type() == QNetworkInterface::Ethernet) ||
                (iface.type() == QNetworkInterface::Wifi))
            {
                //            nslist << interface.name();
                QList<QNetworkAddressEntry> entryList = iface.addressEntries();
                // only return first address
                if (entryList.length() > 0)
                {
                    QNetworkAddressEntry entry = entryList.at(0);
                    tmp = entry.ip().toString();
                }
                break;
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return tmp;
}

QString QIperfd::getManagerInterface()
{
    QString ifname;
    ifname = mgr_ifname;
#if defined(Q_OS_WIN32)
    ifname = getIfNameByHumanReadableName(mgr_ifname);
#endif
    return ifname;
}

QString QIperfd::getIfNameByHumanReadableName(QString name)
{
    QString ifname="";
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface iface, list) // 遍歷每一個網路介面
    {
        if (name.compare(iface.humanReadableName()) == 0)
        {
            ifname = iface.name();
            break;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return ifname;
}

qint64 QIperfd::add(QString returnAddress, QString refrow, int version,
                    QString m_cmd, QString args, uint port,
                 QString bindaddr, QString target,
                 QString parallel, QString protocal, bool bidir, bool reverse,
                 int interval, qint64 duration,
                 int delaytime, bool bServer, bool ignoreWrongInterval,
                 bool restartonerror, QJsonObject restartrule)
{ // add a IperfWorker to run iperf server/client
    // TODO: check host/port used?
    QThread *iperf_th = new QThread();
    qint64 idx =0;
    if (bServer){
        idx = m_thserver.count();
        m_thserver.insert(idx, iperf_th);
    }else{
        idx = m_threads.count();
        m_threads.insert(idx, iperf_th);
    }
    debug("create IperfWorker", 6);
    IperfWorker *iperfer = new IperfWorker(returnAddress, idx, version, m_cmd,
                                           args, port,
                                           bindaddr, target, bidir, reverse,
                                           interval, duration, delaytime,
                                           ignoreWrongInterval, restartonerror,
                                           restartrule, tmpfilepath+QDir::separator());
    debug("IperfWorker created", 6);
    iperfer->setRefRow(refrow);
    iperfer->setExtra(parallel, protocal, port);
//    connect(iperfer, &IperfWorker::onStdout, this, &QIperfd::readStdOut);
    connect(iperfer, &IperfWorker::onStderr, this, &QIperfd::onErrored);
    connect(iperfer, &IperfWorker::log, this, &QIperfd::onIperfLog);
    connect(iperfer, &IperfWorker::started, this, &QIperfd::onStarted);
    connect(iperfer, &IperfWorker::restarted, this, &QIperfd::onReStarted);
    connect(iperfer, &IperfWorker::finished, this, &QIperfd::onFinished);
    connect(iperfer, &IperfWorker::iperfTPdata, this, &QIperfd::onThroughput);
    connect(iperfer, &IperfWorker::iperfExtendWait, this, &QIperfd::onIperfExtendWait);
    connect(iperfer, &IperfWorker::debuginfo, this, &QIperfd::onDebuginfo);
    connect(iperfer, &IperfWorker::workerFinished, this, &QIperfd::handleWorkerFinished);
    // connect(this, &QIperfd::setStop, iperfer, &IperfWorker::setStop);
    // if (bServer){
    connect(this, &QIperfd::StopServer, iperfer, &IperfWorker::setStop);
    // }else{
    connect(this, &QIperfd::StopClient, iperfer, &IperfWorker::setStop);
    // }
    connect(this, &QIperfd::setDebugLv, iperfer, &IperfWorker::onSetDebugLv);
    connect(this, &QIperfd::setStartTime, iperfer, &IperfWorker::onSetStartTime);
    connect(this, &QIperfd::setReStart, iperfer, &IperfWorker::onSetReStart);

    connect(iperf_th, &QThread::started, iperfer, &IperfWorker::work);
    connect(iperf_th, &QThread::finished, iperf_th, &QThread::deleteLater);
    iperfer->moveToThread(iperf_th);

    if (bServer){
        m_iperfwserver.insert(idx, iperfer);
    }else{
        m_iperfworkers.insert(idx, iperfer);
    }
    return idx;
}

qint64 QIperfd::add(QString returnAddress, QString refrow,
                    QString sIgnoreWrongInterval, QVariantMap jsondata)
{
    // add iperf by jsondata
    int ver = jsondata["version"].toInt();
    QString cmd;
    if (ver == static_cast<int>(IPERF_VER::V3)){
        cmd = m_iperfexe3;
    }else if ((ver==static_cast<int>(IPERF_VER::V2))||
              (ver==static_cast<int>(IPERF_VER::V21))||
              (ver==static_cast<int>(IPERF_VER::V22))
               ){
        cmd = m_iperfexe2;
    }else{
        debug("Not support Iperf version:" + QString::number(ver));
        return -1;
    }
    uint port = jsondata["port"].toUInt();
    bool isServer=false;
    if (jsondata.contains("server")){
        isServer=true;
    }
    QString binaddr = jsondata["bind"].toString();
    QString target = jsondata["target"].toString();
    QString parallel = jsondata["parallel"].toString(); // for server mode use
    QString protocal = jsondata["protocal"].toString(); // for server mode use
    bool bidir = jsondata["bidir"].toBool(); // for server mode use
    bool reverse = jsondata["reverse"].toBool(); // for server mode use
    int interval = jsondata["interval"].toInt();
    int delaytime = jsondata["delaytime"].toInt();
    qint64 duration = 0;
    if (jsondata.contains("duration")){
        duration = jsondata["duration"].toDouble();
    }
    bool ignoreWrongInterval = false;
    if (sIgnoreWrongInterval.startsWith("1")){
        ignoreWrongInterval = true;
    }
    bool restartonerror = jsondata["restartonerror"].toBool();
    debug(QString("restartonerror:%1").arg(restartonerror?"true":"false"), 5);
    QJsonObject restartruleObject = QJsonObject();
    if (restartonerror){
        QVariantMap restartrule;// = QVariantMap();
        restartrule = jsondata["restartrule"].toMap();
        restartruleObject = QJsonObject::fromVariantMap(restartrule);
        // debug use
        // QJsonDocument doc(restartruleObject);
        // QString jsonString = doc.toJson(QJsonDocument::Indented);
        // debug("restartrule: "+jsonString, 5);
    }

    //conver json data format to iperf args
    QString args;
    if (ver == static_cast<int>(IPERF_VER::V3)){
        args = m_iperfwrapper->toIperf3args(jsondata);
    }else if ((ver==static_cast<int>(IPERF_VER::V2))||
               (ver==static_cast<int>(IPERF_VER::V21))||
               (ver==static_cast<int>(IPERF_VER::V22))
               ){
        args = m_iperfwrapper->toIperf2args(jsondata);
    }else {
        debug("Not support Iperf version:" + QString::number(ver), 4);
        return -1;
    }
    debug("args:" + args, 5);
    return add(returnAddress, refrow, ver, cmd, args, port, binaddr,
               target, parallel, protocal, bidir, reverse, interval, duration,
               delaytime,
               isServer, ignoreWrongInterval,
               restartonerror, restartruleObject);
}

void QIperfd::del(int idx, bool servermode)
{
    //delete specify m_iperfworkers & m_threads
    //
    if (servermode){
        if (m_thserver.contains(idx)){
            m_thserver.remove(idx);
        }
        if (m_iperfwserver.contains(idx)){
            m_iperfwserver.remove(idx);
        }
    }else{
        if (m_threads.contains(idx)){
            m_threads.remove(idx);
        }
        if (m_iperfworkers.contains(idx)){
            m_iperfworkers.remove(idx);
        }

        m_runstatus[idx]=0;
    }
}

// int QIperfd::addIperfServer(QString refrow, int version, uint port, QString bindHost)
// {
//     // add a Iperf Server
//     debug("addIperfServer:" + bindHost + ":" + QString::number(port), 4);

//     QString cmd;
//     QString argbind;
//     // add a iperf server
//     if (version==static_cast<int>(IPERF_VER::V3)){
//         cmd = m_iperfexe3;
//         argbind = " --bind ";
//     }else if ((version==static_cast<int>(IPERF_VER::V2))||
//                (version==static_cast<int>(IPERF_VER::V21))||
//                (version==static_cast<int>(IPERF_VER::V22))
//                ){
//         cmd = m_iperfexe2;
//         argbind = " -B ";
//     }else{
//         debug("Not support Iperf version:" + QString::number(version), 4);
//         return -1;
//     }
//     QString args=" -s ";
//     if (!(bindHost=="")) {
//         args.append(argbind);
//         args.append(bindHost);
//     }
//     return add(refrow, version, cmd, args, port, bindHost);
// }

// int QIperfd::addIperfClient(QString refrow, int version, uint port, QString Host, QString iperfargs)
// {
//     // add a Iperf Client
//     debug("addIperfClient:" + Host + ":" + QString::number(port));
//     QString cmd;
//     // add a iperf server
//     if (version==static_cast<int>(IPERF_VER::V3)){
//         cmd = m_iperfexe3;
//     }else if ((version==static_cast<int>(IPERF_VER::V2))||
//                (version==static_cast<int>(IPERF_VER::V21))||
//                (version==static_cast<int>(IPERF_VER::V22))
//                ){
//         cmd = m_iperfexe2;
//     }else{
//         debug("Not support Iperf version:" + QString::number(version), 4);
//         return -1;
//     }
//     QString args = iperfargs;
//     return add(refrow, version, cmd, args, port);
// }

void QIperfd::startServer(int idx)
{
    //start iperf server
    QThread *th = m_thserver.value(idx);
    if (!th) {
        debug("startServer: QThread is null", 3);
        return;
    }
    if (th->isRunning() ){
        debug( "startServer m_thserver(" + QString::number(idx) + ") already running", 3);
        return;
    }
    IperfWorker *worker = m_iperfwserver.value(idx);
    if (!worker) {
        debug("startServer: IperfWorker is null", 3);
        return;
    }
    // if (worker->isRunning()) {
    //     debug( "m_iperfwserver(" + QString::number(idx) + ") already running", 3);
    //     return;
    // }
    debug(" start iperfworkers server:" + QString::number(idx), 2);
    worker->setIperfLogPath(tmpfilepath + QDir::separator() + s_starttime);
    th->start();
}

void QIperfd::start(int idx)
{
    //start iperf client
    QThread *th = m_threads.value(idx);
    if (!th) {
        debug("start client: QThread is null");
        return;
    }
    if (th->isRunning()){
        debug( "start m_threads(" + QString::number(idx) + ") already running");
        return;
    }

    IperfWorker *worker = m_iperfworkers.value(idx);
    if (!worker) {
        debug("start client: IperfWorker is null");
        return;
    }
    if (worker->isRunning()) {
        debug( "m_iperfworkers(" + QString::number(idx) + ") already running");
        return;
    }
    worker->setIperfLogPath(tmpfilepath + QDir::separator() + s_starttime);
    debug(" start iperfworkers client:" + QString::number(idx), 2);
    th->start();
}

QString QIperfd::longLongListToString(const QList<long long int>& list, const QString& separator) {
    QStringList stringList;
    for (long long int value : list) {
        stringList.append(QString::number(value)); // Convert each long long int to QString
    }
    return stringList.join(separator); // Join all QStrings with the specified separator
}

void QIperfd::startAll(bool bServer)
{
    QString tmp = tmpfilepath+QDir::separator()+s_starttime;
    QDir d(tmp);
    if (!d.exists()){
        d.mkpath(tmp);
    }
    //TODO: this will not set logpath at right time
    emit setStartTime(s_starttime);
    // start all thread
    if (bServer){
        for (auto it = m_thserver.begin(); it != m_thserver.end(); ++it)
        {
            startServer(it.key());
            // QCoreApplication::processEvents(QEventLoop::AllEvents);// do not add this, cause things run in
        }
    }else{
        for (auto it = m_threads.begin(); it != m_threads.end(); ++it)
        {
            start(it.key());
            // QCoreApplication::processEvents(QEventLoop::AllEvents);// do not add this ?
        }
    }
}
void QIperfd::stop(int idx)
{   //stop runing iperf
    if (isRunning(idx)){
        m_iperfworkers.value(idx)->setStop();
        //    iperfwork->setStop();
    }
}

void QIperfd::stopAll()
{
    // emit setStop();
    debug("emit StopClient()");
    emit StopClient();
    debug("emit StopServer()");
    emit StopServer();
}

void QIperfd::clear()
{
    //clear all m_iperfwserver/m_iperfworkers & m_threads
    if (!m_iperfwserver.isEmpty()) {
        emit StopServer();
        auto it = m_iperfwserver.begin();
        while (it != m_iperfwserver.end()) {
            debug("[QIperfd::clear]delete iperf server worker", 3);
            IperfWorker* obj = it.value();
            if (obj->getPID()){
                //still running
                obj->setStop();
            }
            delete obj; // Ensure value is valid
            it = m_iperfwserver.erase(it);
        }
    }

    if (!m_thserver.isEmpty()) {
        auto it = m_thserver.begin();
        while (it != m_thserver.end()) {
            if (it.value()) {
                it.value()->quit();
                debug("[QIperfd::clear]wait server thread stop", 3);
                it.value()->wait(5000);
                debug("[QIperfd::clear]wait server thread stopped", 3);
            }
            it = m_thserver.erase(it);
        }
    }

    if (!m_iperfworkers.isEmpty()) {
        emit StopClient();
        auto it = m_iperfworkers.begin();
        while (it != m_iperfworkers.end()) {
            debug("[QIperfd::clear]delete iperf client worker", 3);
            IperfWorker* obj = it.value();
            if (obj->getPID()){
                //still running
                obj->setStop();
            }
            delete obj;
            it = m_iperfworkers.erase(it);
        }
    }

    if (!m_threads.isEmpty()) {
        auto it = m_threads.begin();
        while (it != m_threads.end()) {
            if (it.value()) {
                it.value()->quit();
                debug(QString("[QIperfd::clear]wait client thread stop %1").arg(it.key()), 3);
                it.value()->wait(8000);
                debug("[QIperfd::clear]wait client thread stopped", 3);
            }
            it = m_threads.erase(it);
        }
    }

    debug("iperfserver:" + QString::number(m_iperfwserver.count())
          + " threads:" + QString::number(m_thserver.count())
          + " iperfclient:" + QString::number(m_iperfworkers.count())
          + " thread:" + QString::number(m_threads.count()));
}

bool QIperfd::isRunning(int idx)
{
    if (m_iperfworkers.contains(idx)) {
        return m_iperfworkers.value(idx)->isRunning();
    }
    return false;
}

void QIperfd::restartQIperfd()
{
    infoQIperfdStopped();
    QTimer::singleShot(1500, this, SLOT(doRestartQIperfd()));
}

void QIperfd::infoQIperfdStopped()
{
    if (m_udpsrv){
        m_udpsrv->setSendMsg(m_myinfo->disableInfo()); // broadcast
    }
}

void QIperfd::informMessage(QString data, bool bShowAtLocal)
{
    //data : json format of string or simple string info with "："
    if (bShowAtLocal){
        m_pserver->sendMessage(data);
    }
    m_wsserver->sendTextResult(data);
}

void QIperfd::setManagerInterface(QString ifname)
{
    mgr_ifname = ifname;
    savecfg();
    emit setMgrIfname(ifname);
    m_udpsrv->setSendMsg(m_myinfo->updateInfo()); // broadcast
}

void QIperfd::onPipeMessage(int idx, const QString msg)
{
//    onLog("(" + QString(idx) + ")onPipeMessage: = " + msg);
    if (QString::compare(msg, CMD_OK, Qt::CaseInsensitive) == 0)
    {
        return;
    }
    if (QString::compare(msg, CMD_ARGS, Qt::CaseInsensitive) == 0)
    {
        onLog("accept args from anther qiperd: " + msg );
        return;
    }
    if (QString::compare(msg, CMD_STATUS, Qt::CaseInsensitive) == 0)
    {
        // get current all iperf status
        QVariantMap status;
        QVariantMap workers;
        workers.insert("iperfworkers", QString::number(m_iperfworkers.count()));
        status.insert("CMD", CMD_STATUS);
        status.insert(CMD_STATUS, workers);
        status.insert(QIPERFD_NAME, QIPERFD_VERSION);
        // TODO: any iperf running
        status.insert(CMD_RUNNING, QString::number(m_iperfworkers.count()));
        // NTP server
        status.insert(CMD_NTP_START, bNtpserver);
        QJsonDocument jsonDocument = QJsonDocument::fromVariant(status);
        QString backmsg = jsonDocument.toJson(QJsonDocument::Compact).toStdString().c_str();
        informMessage(backmsg, true);
    }
    else if (QString::compare(msg, CMD_IFNAMES, Qt::CaseInsensitive) == 0)
    {
        // get all interfaces names
        QJsonObject netObjs = m_myinfo->collectNetInfo();
        QVariantMap status;
        QVariantMap ifnames;
        ifnames.insert("ifnames", netObjs.keys());
        debug("CMD_IFNAMES:" + QString(QJsonDocument(netObjs).toJson()), 4);
        status.insert("CMD", CMD_IFNAMES);
        status.insert(CMD_IFNAMES, ifnames);
        status.insert("ifname", mgr_ifname); // current manager ifname
        status.insert("netobj", QString(QJsonDocument(netObjs).toJson()));
        QJsonDocument jsonDocument = QJsonDocument::fromVariant(status);
        QString backmsg = jsonDocument.toJson(QJsonDocument::Compact).toStdString().c_str();
        informMessage(backmsg, true);
    }
    else if (QString::compare(msg, CMD_QIPERFD_RESTART, Qt::CaseInsensitive) == 0)
    {
        debug("CMD_QIPERFD_RESTART: " + msg, 4);
        restartQIperfd();
    }
    else if (QString::compare(msg, CMD_GET_LOGFILENAME, Qt::CaseInsensitive) == 0)
    {
        QString backmsg = QString(CMD_GET_LOGFILENAME)+"："+ qiperfdlog;
        informMessage(backmsg, true);
    }
    else if (QString::compare(msg, CMD_IPERFVER, Qt::CaseInsensitive) == 0)
    {   //get iperf version
        QJsonObject jobj = m_myinfo->getIperfVer();
        informMessage(QString(CMD_IPERFVER)+"："+QString(QJsonDocument(jobj).toJson()), true);
    }
    else if (msg.startsWith(CMD_NTP_START, Qt::CaseInsensitive))
    {
        long long cut = msg.indexOf(':');
        QString ntpmode = msg.right(msg.length()-cut-1);
        debug("CMD_NTP_START:ntpmode:" + ntpmode, 4);
        setNtpServer(ntpmode);
    }
    else
    {
        debug("handle json: " + msg, 2);
        // json format message
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError)
        {
            QVariantMap result = doc.toVariant().toMap();
            QString act = result["Action"].toString();
            if (QString::compare(act, CMD_SET_IFNAME, Qt::CaseInsensitive) == 0)
            {
                QString ifname = result[CMD_SET_IFNAME].toString();
                setManagerInterface(ifname);
            }
            else if (QString::compare(act, CMD_IPERF_ADD, Qt::CaseInsensitive) == 0)
            {
                //iperf_args is in iperf native args format
                QVariantMap iperf_args = result["iperf"].toMap();
//                add(result["iperf"].toMap());
                int ver = iperf_args["version"].toInt();
                QString cmd;
                if (ver == static_cast<int>(IPERF_VER::V3)) {
                    cmd = m_iperfexe3;
                } else {
                    cmd = m_iperfexe20;
                }
                uint port = iperf_args["port"].toUInt();
                QString args;
                foreach (QVariant arg, iperf_args["cmd_args"].toList())
                {
                    onLog("arg: " + arg.toString());
                    args = " " + arg.toString();
                }
                onLog("add iperf: " + args);
                // TODO: onPipeMessage refrow,
                add("", "0", ver, cmd, args, port);
            }
            else if (QString::compare(act, CMD_IPERF_START, Qt::CaseInsensitive) == 0)
            {
                onLog("Start all iperfs");
                startAll();
            }
            else if (QString::compare(act, CMD_IPERF_STOP, Qt::CaseInsensitive) == 0)
            {
                onLog("Stop all iperfs");
                stopAll();
            }
            else
            {
                onLog("TODO: handle new json message:(" + QString::number(idx) + ")" + (msg));
            }
        }
        else
        {
            //            qFatal(error.errorString().toUtf8().constData());
            onLog("onNewMessage: ERROR: " + error.errorString() + "\nparser json: " + msg.toUtf8());
            exit(1);
        }
    }
}

void QIperfd::readStdOut(int idx, QString text)
{
    onLog("TODO: readStdOut(" + QString::number(idx) + "):" + text);
}

void QIperfd::onErrored(int m_idx, int refrow, QString text, QString ipport)
{
    Q_UNUSED(m_idx)
    QString msg = QString(CMD_IPERF_ERRORED)+":"+QString::number(refrow);
    msg = msg + ":1:"+ text+":"+ipport; // error no, error
    m_wsserver->sendTextResult(msg);
    //remove error process??
    // if (isRunning(m_idx)){
    //     stop(m_idx);
    // }
    // del(m_idx);
}

void QIperfd::onIperfLog(int idx, QString text)
{
    onLog("(" + QString::number(idx) + ")" + text + "");
}

void QIperfd::onStarted(int m_idx, bool smode, QString ipport)
{
    QString msg = QString(CMD_IPERF_STARTED)+":"+QString::number(m_idx);
    if (smode){
        msg = msg + ":S";
    }else{
        msg = msg + ":C";
    }
    msg = msg + ":"+ ipport;
    debug("onStarted: " + msg, 4);
    m_wsserver->sendTextResult(msg);
    m_runstatus[m_idx]=1;
}

void QIperfd::onReStarted(int m_idx, bool smode, QString ipport)
{
    //CMD_IPERF_RESTARTED:m_idx:smode:ipport
    QString msg = QString(CMD_IPERF_RESTARTED)+":"+QString::number(m_idx);
    if (smode){
        msg = msg + ":S";
    }else{
        msg = msg + ":C";
    }
    msg = msg + ":"+ ipport;
    debug("onReStarted: " + msg, 2);
    m_wsserver->sendTextResult(msg);
}

void QIperfd::onFinished(int refrow, int exitCode, int exitStatus, QString ipport, QString filename, bool servermode)
{
    QString msg = QString(CMD_IPERF_STOPED)+":"+ QString::number(refrow);
    msg = msg + ":" + QString::number(exitCode)+  ":" + QString::number(exitStatus);
    msg = msg + ":" + ipport;
    onLog("onFinished: " + msg + " server:" + QString::number(servermode));
    m_wsserver->sendTextResult(msg);
    if (!filename.isEmpty()){
        if(QFileInfo::exists(filename)){
            m_fileclient->enqueueFile(filename);
        }else{
            onLog("file to send not Exist: " + filename);
        }
    }
    del(refrow, servermode);

}

void QIperfd::onThroughput(QString returnAddress, int idx, QString sInterval, QString data)
{
    QString tp_data = QString(CMD_IPERF_TP_DATA) + ":" + QString::number(idx)
                + ":" + sInterval + ":" + data;
    // m_wsserver->sendTextResult(tp_data);
    m_wsserver->sendTextMessage(tp_data, returnAddress);
}

void QIperfd::onIperfExtendWait(int refrow, qint64 iwait, int exitCode, int restarttimes)
{
    QString s = QString(CMD_IPERF_EXTEND_WAIT) + ":" + QString::number(refrow)
                + ":" + QString::number(iwait) + ":" + QString::number(exitCode)
                + ":" + QString::number(restarttimes);
    onLog("onIperfExtendWait: " + s);
    m_wsserver->sendTextResult(s);
}

void QIperfd::onQuit()
{
    onLog("onQuit");
    savecfg();
    qApp->quit();
}

void QIperfd::onNewLine(QString line)
{
    //pipe server sendMessage
    m_pserver->sendMessage(QIPERFDLOG+QString("：")+line);
}

void QIperfd::doRestartQIperfd()
{
    debug("doRestartQIperfd", 4);
#if defined(Q_OS_LINUX)
#if !defined(Q_OS_ANDROID)
    // cmd = "systemctl restart qiperfd";
    const QStringList arguments ={"restart", "qiperfd"};
    qApp->quit();
    QProcess::startDetached("systemctl", arguments);
#else
    debug("TODO: restart android qiperfd service", 2)
#endif
#elif defined(Q_OS_WINDOWS)
    QString taskName = "startqiperfd";
    // Set task to run 1 sec from now
    QDateTime runTime = QDateTime::currentDateTime().addSecs(3);
    //nssm.exe restart "qiperfd"
    QString taskCommand = "\"" + m_nssm + "\"";
    QString taskArgs = "restart qiperfd";
    if (createScheduledTask(taskName, taskCommand, taskArgs, runTime)) {
        debug("Task created successfully.", 4);
    } else {
        debug("Failed to create task.", 2);
    }
#else
    debug("do Restart QIperfd for system :" + QSysInfo::productType(), 2);
#endif
}

void QIperfd::onSerialTaskFinished(QString serialPortName)
{
    Q_UNUSED(serialPortName)
    //only receive CMD_SERIAL_DEL do serial close

}

void QIperfd::onSerialTaskStarted(QString idx, quint16 port)
{
    informMessage(QString("%1:%2:%3").arg(CMD_SERIAL_OPENED, idx, QString::number(port)));
}

void QIperfd::onSerialTaskError(QString idx, QString errormsg)
{
    informMessage(QString("%1:%2:%3").arg(CMD_SERIAL_FAIL, idx, errormsg));
}

void QIperfd::onSSHTaskFinished(QString target)
{
    Q_UNUSED(target)
    //TODO: onSSHTaskFinished
}

void QIperfd::onSSHTaskStarted(QString idx, quint16 port)
{
    informMessage(QString("%1:%2:%3").arg(CMD_SSH_OPENED, idx, QString::number(port)));
}

void QIperfd::onSSHTaskError(QString idx, QString errormsg)
{
    debug(idx + " onSSHTaskError:" + errormsg, 3);
    informMessage(QString("%1:%2:%3").arg(CMD_SSH_FAIL, idx, errormsg));
}

void QIperfd::onTimeSynced(QString target, bool synced)
{
    Q_UNUSED(target)
    QString cmd="";
    int rc;
    if (synced){
        cmd = QString("%1").arg(CMD_NTP_SYNC_OK);
    }else{
        cmd = QString("%1").arg(CMD_NTP_SYNC_FAIL);
    }
    rc = m_wsserver->sendTextMessage(cmd);
    if (rc<=0){
        debug(QString("error onTimeSynced synced:%1 send cmd fail: %2").arg(synced?"true":"false", cmd) , 2);
    }
}

#if defined(Q_OS_WINDOWS)
QString QIperfd::comErrorToString(HRESULT hr) {
    _com_error err(hr);
    LPCTSTR errMsg = err.ErrorMessage();
    return QString::fromWCharArray(errMsg);
}
// --- COM API Implementations ---

// Helper function to convert QString to BSTR (used by COM)
// Qt's QString::toStdWString() is good, then convert to BSTR
_bstr_t toBSTR(const QString& s) {
    return _bstr_t(s.toStdWString().c_str());
}

bool QIperfd::createScheduledTask(const QString &taskName, const QString &taskCommand, const QString &taskArgs, const QDateTime &runTime) {
    HRESULT hr = S_OK;

    // Use _com_ptr_t for automatic reference counting and error checking
    // Use raw COM interface pointers
    ITaskService* pService = nullptr;
    ITaskFolder* pRootFolder = nullptr;
    ITaskDefinition* pTask = nullptr;
    IRegistrationInfo* pRegInfo = nullptr;
    IActionCollection* pActionCollection = nullptr;
    IExecAction* pExecAction = nullptr;
    ITriggerCollection* pTriggerCollection = nullptr;
    ITimeTrigger* pTimeTrigger = nullptr;
    ITaskSettings* pSettings = nullptr;
    IPrincipal* pPrincipal = nullptr;
    IRegisteredTask* pRegisteredTask = nullptr; // Fixed: was IRegisteredTaskPtr
    IAction* pAction = nullptr;
    ITrigger* pTrigger = nullptr;

    try {
        // 2. Create a TaskService instance
        // hr = pService->CreateInstance(CLSID_TaskScheduler);
        // Create instance:
        hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER,
                              IID_ITaskService, (void**)&pService);
        if (FAILED(hr)) _com_issue_error(hr);

        // 3. Connect to the Task Scheduler service
        hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
        if (FAILED(hr)) _com_issue_error(hr);

        // 4. Get the root task folder
        hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
        if (FAILED(hr)) _com_issue_error(hr);

        // Remove the task if it already exists (optional, but good for testing)
        pRootFolder->DeleteTask(toBSTR(taskName), 0); // Ignore error if task doesn't exist

        // 5. Create a new task definition
        hr = pService->NewTask(0, &pTask);
        if (FAILED(hr)) _com_issue_error(hr);

        // 6. Set task registration information
        hr = pTask->get_RegistrationInfo(&pRegInfo);
        if (FAILED(hr)) _com_issue_error(hr);
        hr = pRegInfo->put_Author(toBSTR(QString("QtApp (%1)").arg(QCoreApplication::applicationName())));
        if (FAILED(hr)) _com_issue_error(hr);
        hr = pRegInfo->put_Description(toBSTR(QString("Task created by %1 for testing '%2'").arg(QCoreApplication::applicationName(), taskCommand)));
        if (FAILED(hr)) _com_issue_error(hr);

        // 7. Define an action (e.g., execute a program)
        hr = pTask->get_Actions(&pActionCollection);
        if (FAILED(hr)) _com_issue_error(hr);
        hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
        if (FAILED(hr)) _com_issue_error(hr);
        hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction); // Query to specific interface
        if (FAILED(hr)) _com_issue_error(hr);
        // Note: IID_IExecAction needs to be defined by #import or manually
        // If it's not working, ensure your #import path is correct and taskschd.tlh is generated.

        // Set the path to the executable
        hr = pExecAction->put_Path(toBSTR(taskCommand));
        if (FAILED(hr)) _com_issue_error(hr);
        // If your command needs arguments, use put_Arguments(toBSTR("arg1 arg2"));
        hr = pExecAction->put_Arguments(toBSTR(taskArgs));
        if (FAILED(hr)) _com_issue_error(hr);

        // 8. Define a trigger (e.g., a time trigger)
        hr = pTask->get_Triggers(&pTriggerCollection);
        if (FAILED(hr)) _com_issue_error(hr);
        // Use ITrigger* instead of IDispatch*
        hr = pTriggerCollection->Create(TASK_TRIGGER_TIME, &pTrigger);
        if (FAILED(hr)) _com_issue_error(hr);
        hr = pTrigger->QueryInterface(IID_ITimeTrigger, (void**)&pTimeTrigger);
        if (FAILED(hr)) _com_issue_error(hr);

        // Set the start boundary (when the trigger becomes active)
        // Format: YYYY-MM-DDTHH:MM:SSTZ (e.g., "2025-06-18T08:30:00")
        QString startTime = runTime.toString("yyyy-MM-ddThh:mm:ss");
        hr = pTimeTrigger->put_StartBoundary(toBSTR(startTime));
        if (FAILED(hr)) _com_issue_error(hr);
        // For a one-time trigger, usually omit put_EndBoundary or set to start boundary + epsilon.

        // 9. Set task settings (optional but good practice)
        hr = pTask->get_Settings(&pSettings);
        if (FAILED(hr)) _com_issue_error(hr);
        hr = pSettings->put_Enabled(VARIANT_TRUE); // Enable the task
        if (FAILED(hr)) _com_issue_error(hr);
        hr = pSettings->put_Hidden(VARIANT_FALSE); // Make it visible in Task Scheduler UI
        if (FAILED(hr)) _com_issue_error(hr);
        hr = pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE); // Don't stop on battery
        if (FAILED(hr)) _com_issue_error(hr);
        hr = pSettings->put_WakeToRun(VARIANT_TRUE); // Wake computer to run task
        if (FAILED(hr)) _com_issue_error(hr);


        // 10. Set principal (user context) - REQUIRED for creating tasks
        hr = pTask->get_Principal(&pPrincipal);
        if (FAILED(hr)) _com_issue_error(hr);
        // Set to run as 'SYSTEM' or 'Interactive' for current user, 'Highest Run Level' for admin.
        // For 'Highest Run Level' it's often best to omit put_UserId and put_LogonType.
        // If you need a specific user, use put_UserId and put_LogonType(TASK_LOGON_PASSWORD)
        // and register with a password.
        hr = pPrincipal->put_UserId(_bstr_t(L"SYSTEM"));
        if (FAILED(hr)) _com_issue_error(hr);
        hr = pPrincipal->put_LogonType(TASK_LOGON_SERVICE_ACCOUNT);
        if (FAILED(hr)) _com_issue_error(hr);
        hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST); // This typically requires Admin rights for your app.
        if (FAILED(hr)) _com_issue_error(hr);


        // 11. Register the task
        // TASK_CREATE_OR_UPDATE flag will overwrite existing task of the same name.
        _variant_t password = _variant_t(); // No password if using SYSTEM or INTERACTIVE_TOKEN
        hr = pRootFolder->RegisterTaskDefinition(
            toBSTR(taskName),      // Task Name
            pTask,                // Task Definition
            TASK_CREATE_OR_UPDATE, // Flags: create or update
            _variant_t(),           // No user specified
            _variant_t(),           // No password
            TASK_LOGON_NONE,        // ✅ Change this to TASK_LOGON_NONE
            _variant_t(),           // No SDDL
            &pRegisteredTask      // Output: Registered task object
            );
        if (FAILED(hr)) _com_issue_error(hr);

        debug(QString("Task '%1' registered successfully.").arg(taskName), 4);
        // Cleanup resources
        if (pRegisteredTask) pRegisteredTask->Release();
        if (pTrigger) pTrigger->Release();
        if (pAction) pAction->Release();
        if (pPrincipal) pPrincipal->Release();
        if (pSettings) pSettings->Release();
        if (pTimeTrigger) pTimeTrigger->Release();
        if (pTriggerCollection) pTriggerCollection->Release();
        if (pExecAction) pExecAction->Release();
        if (pActionCollection) pActionCollection->Release();
        if (pRegInfo) pRegInfo->Release();
        if (pTask) pTask->Release();
        if (pRootFolder) pRootFolder->Release();
        if (pService) pService->Release();
        return true;

    } catch (const _com_error& error) {
        debug(QString("COM Error during task creation: %1 (HRESULT: 0x%2)")
                  .arg(comErrorToString(error.Error()))
                  .arg(error.Error(), 8, 16, QChar('0').toUpper()), 2);
        // Cleanup on error
        if (pRegisteredTask) pRegisteredTask->Release();
        if (pTrigger) pTrigger->Release();
        if (pAction) pAction->Release();
        if (pPrincipal) pPrincipal->Release();
        if (pSettings) pSettings->Release();
        if (pTimeTrigger) pTimeTrigger->Release();
        if (pTriggerCollection) pTriggerCollection->Release();
        if (pExecAction) pExecAction->Release();
        if (pActionCollection) pActionCollection->Release();
        if (pRegInfo) pRegInfo->Release();
        if (pTask) pTask->Release();
        if (pRootFolder) pRootFolder->Release();
        if (pService) pService->Release();
        return false;
    } catch (...) {
        debug("An unknown error occurred during task creation.", 2);
        // Cleanup on error
        if (pRegisteredTask) pRegisteredTask->Release();
        if (pTrigger) pTrigger->Release();
        if (pAction) pAction->Release();
        if (pPrincipal) pPrincipal->Release();
        if (pSettings) pSettings->Release();
        if (pTimeTrigger) pTimeTrigger->Release();
        if (pTriggerCollection) pTriggerCollection->Release();
        if (pExecAction) pExecAction->Release();
        if (pActionCollection) pActionCollection->Release();
        if (pRegInfo) pRegInfo->Release();
        if (pTask) pTask->Release();
        if (pRootFolder) pRootFolder->Release();
        if (pService) pService->Release();
        return false;
    }
}

bool QIperfd::runScheduledTask(const QString &taskName) {
    HRESULT hr = S_OK;

    ITaskService* pService = nullptr;
    ITaskFolder* pRootFolder = nullptr;
    IRegisteredTask* pRegisteredTask = nullptr;
    IRunningTask* pRunningTask = nullptr;

    try {
        // hr = pService.CreateInstance(CLSID_TaskScheduler);
        hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER,
                              IID_ITaskService, (void**)&pService);
        if (FAILED(hr)) _com_issue_error(hr);

        hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
        if (FAILED(hr)) _com_issue_error(hr);

        hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
        if (FAILED(hr)) _com_issue_error(hr);

        hr = pRootFolder->GetTask(toBSTR(taskName), &pRegisteredTask);
        if (FAILED(hr)) _com_issue_error(hr);

        _variant_t emptyParams; // Empty parameters
        hr = pRegisteredTask->Run(emptyParams, &pRunningTask);
        if (FAILED(hr)) _com_issue_error(hr);

        return true;

    } catch (const _com_error& error) {
        debug(QString("COM Error during task run: %1 (HRESULT: 0x%2)")
                       .arg(comErrorToString(error.Error()))
                  .arg(error.Error(), 8, 16, QChar('0').toUpper()), 4);
        return false;
    } catch (...) {
        debug("An unknown error occurred during task run.", 4);
        return false;
    }
}

bool QIperfd::deleteScheduledTask(const QString &taskName) {
    HRESULT hr = S_OK;
    ITaskService* pService = nullptr;
    ITaskFolder* pRootFolder = nullptr;

    try {
        // hr = pService.CreateInstance(CLSID_TaskScheduler);
        hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER,
                              IID_ITaskService, (void**)&pService);
        if (FAILED(hr)) _com_issue_error(hr);

        hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
        if (FAILED(hr)) _com_issue_error(hr);

        hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
        if (FAILED(hr)) _com_issue_error(hr);

        // Delete the task. Pass 0 for flags (no special options).
        hr = pRootFolder->DeleteTask(toBSTR(taskName), 0);
        if (FAILED(hr)) _com_issue_error(hr); // Will throw if task not found

        return true;

    } catch (const _com_error& error) {
        // ERROR_FILE_NOT_FOUND (0x80070002) is common if the task doesn't exist
        if (error.Error() == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            debug(QString("Task '%1' not found, nothing to delete.").arg(taskName), 2);
            return true; // Consider it successful deletion if it wasn't there
        }
        debug(QString("COM Error during task deletion: %1 (HRESULT: 0x%2)")
                       .arg(comErrorToString(error.Error()))
                  .arg(error.Error(), 8, 16, QChar('0').toUpper()), 2);
        return false;
    } catch (...) {
        debug("An unknown error occurred during task deletion.", 2);
        return false;
    }
}

#endif
void QIperfd::onWSactMessage(QString msg, QHostAddress fromAddr, quint16 fromPort)
{
    QString target = QString("%1:%2").arg(fromAddr.toString(), QString::number(fromPort));
    //handle act message from websocket
    long long cut = msg.indexOf(':', 0);
    QString act = msg.left(cut); //action
    // debug("onWSactMessage: " + act, 3);
    msg = msg.right(msg.length()-cut-1);
    // expect in json format
    if (act.startsWith(CMD_IPERF_ADD)){
        QJsonParseError error;
        cut = msg.indexOf(':', 0);
        QString refrow = msg.left(cut); //refrow number
        msg = msg.right(msg.length()-cut-1);
        cut = msg.indexOf(':', 0);
        QString ignoreWrongInterval = msg.left(cut); //ignoreWrongInterval
        msg = msg.right(msg.length()-cut-1);
        debug("CMD_IPERF_ADD: " + refrow +
                  " ignoreWrongInterval:" + ignoreWrongInterval +
                  " msg: " + msg, 4);

        QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError){
            qint64 rc = add(target, refrow, ignoreWrongInterval,
                            doc.toVariant().toMap());
            debug("iperf idx: "+ QString::number(rc),5);
        }else{
            onLog("onWSactMessage: ERROR: " + error.errorString() + "\nparser json: " + msg.toUtf8());
        }
    }else if (act.startsWith(CMD_IPERF_DEL)){
        onLog("TODO: onWSactMessage: CMD_IPERF_DEL:" + msg);
    }else if (act.startsWith(CMD_IPERF_REG)){
        QStringList d = msg.split(":");
        QString starttime = d[0];
        QString tag = d[1];
        QString bindkey = d[2];
        m_directions[starttime][bindkey] = tag; // tag for bidir
        // bReportTPData = true;
        onLog("SET to Report throughput data: " + msg);
    }else if (act.startsWith(CMD_IPERF_UNREG)){
        QStringList d = msg.split(":");
        QString starttime = d[0];
        QString bindkey = d[1];
        // bReportTPData = false;
        m_directions[starttime][bindkey].clear();
        onLog("SET to NOT Report throughput data: " + msg);
    }else if (act.startsWith(CMD_IPERF_CLEAR)){
        clear();
    }else if (act.startsWith(CMD_IPERF_START)){
        cut = msg.indexOf(':', 0);
        s_starttime = msg.left(cut); //start time string
        m_starttime =QDateTime::fromString(s_starttime, DATETIME_NOW_FORMAT);
        // server/client mode
        QString smode = msg.right(msg.length()-cut-1);
        bool bserver=false;
        // s_starttime = msg;
        if (smode.contains("S")){
            bserver=true;
        }
        // onLog(QString("s_starttime: %1 (%2)").arg(s_starttime, bserver?"server":"client"));
        startAll(bserver);
    }else if (act.startsWith(CMD_IPERF_STOP)){
        QString s = QString("%1 target: %2").arg(CMD_IPERF_STOP, msg);
        onLog(s);
        stopAll();
    }else if (act.startsWith(CMD_IPERF_RESTART)){
        // restart iperf server/client
        cut = msg.indexOf(':', 0);
        QString refrow = msg.left(cut); //refrow number
        // server/client mode
        QString smode = msg.right(msg.length()-cut-1);
        onLog(QString("Restart iperf: refrow:%1 (%2)").arg(refrow, smode));
        if (smode.startsWith("C")){
            emit setReStart(false);
        }else{
            emit setReStart(true);
        }

    }else if (act.startsWith(CMD_PING)){
        debug("CMD_PING", 3);
        cut = msg.indexOf(':', 0);
        QString refrow = msg.left(cut);
        msg = msg.right(msg.length()-cut-1);
        m_icmpping = new IcmpPing(refrow, msg, nullptr);
    }else if (act.startsWith(CMD_NTP_START, Qt::CaseInsensitive)){
        long long cut = msg.indexOf(':');
        QString ntpmode = msg.right(msg.length()-cut-1);
        debug("CMD_NTP_START:" + ntpmode, 4);
        setNtpServer(ntpmode);
    }else if (act.startsWith(CMD_NTP_SYNC)){
        cut = msg.indexOf(':', 0);
        msg = msg.right(msg.length()-cut-1);
        QDateTime stime = QDateTime::fromString(msg, DATETIME_NOW_FORMAT);
        QDateTime curtime = QDateTime::currentDateTime();
        qint64 diffSeconds = curtime.secsTo(stime);
        if (abs(diffSeconds)>1){
            debug("===== Do ntp time sync", 4);
            m_ntpsync->sync(fromAddr);
        }else{
            //No need to do NTP sync
#if (TEST_WS==1)
            onLog("===== Info NTP time is OK: " + target);
            int rc = m_wsserver->sendTextMessage(QString("%1").arg(CMD_NTP_SYNC_OK), target);
            if (rc<=0){
                debug(" Info " + target + " Fail!!", 2);
            }
#endif
        }
    }else if (act.startsWith(CMD_SERIAL_ADD)){
        // TODO: create serial and bind to TCP server
        // idx:comport:BaudRate:DataBits:Parity:StopBits:FlowControl
        onLog("CMD_SERIAL_ADD: " + msg);
        QStringList d = msg.split(":");
        if (d.length()==7){
            long long port = 0;
            SerialTask *task = nullptr;
            QString idx = d[0];
            QString comport = d[1];
            QString baudrate = d[2];

            QSerialPort::DataBits databits = static_cast<QSerialPort::DataBits>(d[3].toInt());
            QSerialPort::Parity parity = static_cast<QSerialPort::Parity>(d[4].toInt());
            QSerialPort::StopBits stopbits = static_cast<QSerialPort::StopBits>(d[5].toInt());
            QSerialPort::FlowControl flowcontrol = static_cast<QSerialPort::FlowControl>(d[6].toInt());
            if (!m_serialtasks.contains(comport)){ // not exist
                port =  QIPERF_SERIALPORT + m_serialtasks.count();
                task = new SerialTask(idx, comport, baudrate,
                                                  "any", QString::number(port),
                                                  VirtualDeviceTcp::Mode::BINARY,
                                                  databits, parity, stopbits, flowcontrol,
                                                  false, false, this);
                connect(task, &SerialTask::finished, this, &QIperfd::onSerialTaskFinished);
                connect(task, &SerialTask::started,  this, &QIperfd::onSerialTaskStarted);
                m_serialtasks.insert(comport, task);
                QTimer::singleShot(0, task, SLOT(init())); // start it
            } else{
                task = m_serialtasks.value(comport);
                quint16 localport = task->getLocalPort();
                if (task->isRunning()){
                    debug(comport + " exist!! Using port:" + QString::number(localport), 4);
                    // TODO: update setting?
                    task->setConfig(QString::number(localport), baudrate,
                                    databits, parity, stopbits, flowcontrol);
                    task->close();
                    QTimer::singleShot(0, task, SLOT(init())); // start it
                    onSerialTaskStarted(idx, localport);
                }else{
                    QString emsg = task->getLastError();
                    debug("SerialTask is not running: " + emsg, 3);
                    onSerialTaskError(idx, emsg);
                }
            }
        }else{
            debug(" Wrong format of create serial: " + msg, 2);
        }
    }else if (act.startsWith(CMD_SERIAL_DEL)){
        QStringList d = msg.split(":");
        if (d.length()==2){
            QString comport = d[1];
            if (m_serialtasks.contains(comport)){
                SerialTask *task = m_serialtasks.value(comport);
                QString idx = task->getIdx();
                task->close();
                if (m_serialtasks.remove(comport)){
                    informMessage(QString("%1:%2").arg(CMD_SERIAL_OK, idx));
                }else{
                    informMessage(QString("%1:%2:%3 %4").arg(CMD_SERIAL_FAIL, idx, comport ,"DEL Fail"));
                }
            }else{
                debug(QString("%1 does not in m_serialtasks!! \n").arg(comport), 4);
                informMessage(QString("%1:%2 %3").arg(CMD_SERIAL_FAIL, comport, "Not Exist"));
            }
        }else{
            debug(" Wrong format of delete serial: " + msg, 2);
        }
    }else if (act.startsWith(CMD_SSH_ADD)){
        QStringList d = msg.split(":");
        if (d.length()==7){
            long long port = 0;
            SSHTask *task = nullptr;
            QString idx = d[0];
            QString sshTarget = d[1];
            QString sshPort = d[2];
            QString key = sshTarget + ":" + sshPort;
            QString username = d[3];
            QString password = d[4];
            QString privateKeyFile = d[5];
            int timeout = d[6].toInt();

            debug("idx:" + idx + " sshTarget:" + sshTarget
                      + " sshPort:" + sshPort
                      + " username:" + username
                      + " password:" + password
                      + " privateKeyFile:" + privateKeyFile
                      + " timeout:" + QString::number(timeout), 5);

            if (!m_sshtasks.contains(key)) {
                port = QIPERF_SSHPORT + m_sshtasks.count();
                task = new SSHTask(idx, sshTarget, sshPort,
                                         "any", QString::number(port),
                                         VirtualDeviceTcp::Mode::BINARY,
                                         username, password, privateKeyFile, timeout);
                connect(task, &SSHTask::finished, this, &QIperfd::onSSHTaskFinished);
                connect(task, &SSHTask::started,  this, &QIperfd::onSSHTaskStarted);
                m_sshtasks.insert(key, task);
                QTimer::singleShot(0, task, SLOT(init()));
            } else {
                auto *task = m_sshtasks.value(key);
                if (!task) {
                    debug("SSHTask lookup failed: task is null", 2);
                    return;
                }
                quint16 localport = task->getLocalPort();
                if (task->isRunning()) {
                    debug(sshTarget + " exist!! Using port:" + QString::number(localport), 4);
                    task->close(); // safe to restart
                    QTimer::singleShot(0, task, SLOT(init()));
                    onSSHTaskStarted(idx, localport);
                } else {
                    QString emsg = "SSHTask is not running: " + task->getLastError();
                    debug(emsg, 2);
                    onSSHTaskError(idx, emsg);
                }
            }

            // try{
            //     QString idx = d[0];
            //     QString sshTarget = d[1];
            //     QString sshPort = d[2];
            //     QString key = sshTarget + ":" + sshPort;
            //     QString username = d[3];
            //     QString password = d[4];
            //     QString privateKeyFile = d[5];
            //     int timeout = d[6].toInt();
            //     debug("idx:" + idx + " sshTarget:" + sshTarget
            //           + " sshPort:" + sshPort
            //           + " username:" + username
            //           + " password:" + password
            //           + " privateKeyFile:" + privateKeyFile
            //           + " timeout:" + QString::number(timeout), 5);
            //     //TODO: m_sshtasks's key format?
            //     if (!m_sshtasks.contains(key)){ // not exist
            //         port =  QIPERF_SSHPORT + m_sshtasks.count();
            //         task = new SSHTask(idx, sshTarget, sshPort,
            //                            "any", QString::number(port),
            //                            VirtualDeviceTcp::Mode::BINARY,
            //                            username, password, privateKeyFile, timeout);
            //         connect(task, &SSHTask::finished, this, &QIperfd::onSSHTaskFinished);
            //         connect(task, &SSHTask::started,  this, &QIperfd::onSSHTaskStarted);
            //         m_sshtasks.insert(key, task);
            //         QTimer::singleShot(0, task, SLOT(init())); // start it
            //     } else {
            //         task = m_sshtasks.value(key);
            //         quint16 localport = task->getLocalPort();
            //         if (task->isRunning()){
            //             debug(sshTarget + " exist!! Using port:" + QString::number(localport), 4);
            //             // TODO: update setting?
            //             // task->setConfig(QString::number(localport), baudrate,
            //             //                 databits, parity, stopbits, flowcontrol);
            //             task->close();
            //             QTimer::singleShot(0, task, SLOT(init())); // start it
            //             onSSHTaskStarted(idx, localport);
            //         }else{
            //             QString emsg = "SSHTask is not running: " + task->getLastError();
            //             debug(emsg, 2);
            //             onSSHTaskError(idx, emsg);
            //         }
            //     }
            // } catch (const std::exception &e) {
            //     debug(QString("SSHTask error: %1").arg(e.what()), 2);
            // }
        }else{
            debug(" Wrong format of create ssh: " + msg, 2);
            // onSSHTaskError(idx, QString("Wrong format of create ssh: %1").arg(msg));
        }
    }else if (act.startsWith(CMD_SSH_DEL)){
        // msg format: "127.0.0.1:192.168.0.90:22"
        QStringList d = msg.split(":");
        if (d.length()==3){
            QString target = d[1];
            QString port = d[2];
            QString key = target + ":" + port;
            if (m_sshtasks.contains(key)){
                SSHTask *task = m_sshtasks.value(key);
                QString idx = task->getIdx();
                task->close();
                if (m_sshtasks.remove(key)){
                    informMessage(QString("%1:%2").arg(CMD_SSH_OK, idx));
                }else{
                    onSSHTaskError(idx, QString("%1:%2").arg(key ,"DEL Fail"));
                }
            }else{
                debug(key + " does not in m_sshtasks!! \n", 4);
                // onSSHTaskError(idx, QString("%1:%2").arg(target, "Not Exist"))
            }
        }else{
            debug(" Wrong format of delete ssh: " + msg, 5);
        }
    }else if (act.startsWith(CMD_QIPERFD_RESTART)){
        restartQIperfd();
    }else if (act.startsWith(CMD_DEBUG_LV)){
        emit setDebugLv(msg.toInt());
    }else if (act.startsWith(CMD_REQUEST_EXEC)){
        cut = msg.indexOf(':', 0);
        QString refid = msg.left(cut); //
        msg = msg.right(msg.length()-cut-1);
        cut = msg.indexOf(':', 0);
        QString reqcmd = msg.left(cut);
        msg = msg.right(msg.length()-cut-1);
        runRequest(refid, target, reqcmd, msg);
    }else {
        QString cmd = QString("%1:%2:%3").arg(CMD_NOT_SUPPORT, act, msg);
        debug("send back: "+cmd);
        int rc = m_wsserver->sendTextMessage(cmd, target);
        if (rc<=0){
            debug(" Info " + target + " Fail!!");
        }
    }
}

void QIperfd::onNewClient(QHostAddress addr)
{
    onNewLine("onNewClient: " +addr.toString());
    debug("onNewClient: " + addr.toString(), 4);
    if (m_fileclient){
        debug("m_fileclient exist: " + m_fileclient->getTargetAddress() + " new: " + addr.toString(), 4);
    }
    //TODO: multi file client
    m_fileclient = new FileClient(QIPERF_FILEPORT, addr.toString());
    connect(this, &QIperfd::setDebugLv, m_fileclient, &FileClient::onSetDebugLv);
}

void QIperfd::onDebuginfo(QString msg)
{
    qDebug() << "[IperfWorker]" << msg;
}

void QIperfd::handleWorkerFinished(qint64 id, bool servermode)
{
    QMutexLocker locker(&m_mutex);
    if (servermode){
        if (m_iperfwserver.contains(id)) {
            IperfWorker* sworker = m_iperfwserver.take(id); // Remove from map
            // The worker should have called deleteLater() itself, so no direct delete here.
            Q_UNUSED(sworker)
            debug("Worker ID " + QString::number(id) + " marked as finished and removed from map.");
        }
    }else{
        if (m_iperfworkers.contains(id)) {
            IperfWorker* worker = m_iperfworkers.take(id); // Remove from map
            // The worker should have called deleteLater() itself, so no direct delete here.
            Q_UNUSED(worker)
            debug("Worker ID " + QString::number(id) + " marked as finished and removed from map.");
        }
    }
}

void QIperfd::onSetDebugLv(int lv)
{
    debug("QIperfd::onSetDebugLv:" + QString::number(lv), 0);
    m_debuglv = lv;
}

void QIperfd::debug(QString msg, int lv)
{
    if (lv <= m_debuglv){
        qDebug() << msg;
    }
}

int QIperfd::checkFirewallStatus()
{
/*return:
-1: error happen
 0: No firewall
 1: firewall is active
*/
#if defined(Q_OS_WIN32)
    HRESULT hres;
    QString err ="";
    // Obtain the initial locator to WMI
    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID *)&pLoc);

    if (FAILED(hres)) {
        // QMessageBox::critical(this, "Error", "Failed to create IWbemLocator object.");
        err = "Error: Failed to create IWbemLocator object.";
        informMessage(err, true);
        return -1;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices *pSvc = NULL;

    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object
        &pSvc                    // pointer to IWbemServices proxy
        );

    if (FAILED(hres)) {
        // QMessageBox::critical(this, "Error", "Could not connect to WMI namespace.");
        err = "Error: Could not connect to WMI namespace. ROOT\\CIMV2";
        informMessage(err, true);
        pLoc->Release();
        return -1;
    }

    // Set security levels on the proxy
    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities
        );

    if (FAILED(hres)) {
        // QMessageBox::critical(this, "Error", "Could not set proxy blanket.");
        err = "Error: Could not set proxy blanket.";
        informMessage(err, true);
        pSvc->Release();
        pLoc->Release();
        return -1;
    }

    // Use the IWbemServices pointer to make requests of WMI
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_FirewallProduct"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres)) {
        // QMessageBox::critical(this, "Error", "Query for firewall status failed.");
        err = "Error: Query for firewall status failed.";
        informMessage(err, true);
        pSvc->Release();
        pLoc->Release();
        return -1;
    }

    // Get the data from the query
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
                                       &pclsObj, &uReturn);

        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;

        // Get the value of the DisplayName property
        hr = pclsObj->Get(L"DisplayName", 0, &vtProp, 0, 0);
        QString firewallProduct = QString::fromWCharArray(vtProp.bstrVal);
        // QMessageBox::information(this, "Firewall Status", "Firewall Product: " + firewallProduct);
        err = "Firewall Status: Firewall Product: " + firewallProduct;
        informMessage(err, true);
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    // Cleanup
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    return uReturn;
#else
    QProcess process;
    // process.setProcessChannelMode(QProcess::MergedChannels);
    QString command = "ps";
    QStringList arguments;
    // arguments << "aux" << " | grep -E 'firewalld|ufw|iptables|nft'";
    arguments << "aux";

    // Start the process
#if QT_VERSION < 0x060000  // < 6.0
    process.start(command, arguments);
#else
    command = command + " " +arguments.join(" ");
    process.startCommand(command);
#endif
    if (!process.waitForFinished(5000)){
        debug("Error run cmd: " + command + " Fail");
        debug(QString("(%1)").arg(QString::fromUtf8(process.readAll())));
        return -1;
    }

    // Read the output
    QByteArray output = process.readAllStandardOutput();
    QByteArray errorOutput = process.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        debug("checkFirewallStatus Error:\n" + errorOutput + "\n\n CMD:" + command + "\n");
        return -1;
    } else{
        QString lines = QString::fromUtf8(output);
        foreach (QString line, lines.split("\n")) {
            if ((line.contains("firewalld", Qt::CaseInsensitive))||
                (line.contains("ufw", Qt::CaseInsensitive))||
                (line.contains("iptables", Qt::CaseInsensitive))||
                (line.contains("nft", Qt::CaseInsensitive))){
                return 1;
            }
        }
        return 0;
    }
#endif
}

void QIperfd::setNtpServer(QString mode)
{
    if (mode.startsWith("0")){
        // disable
        bNtpserver = false;
    }else{
        // enable
        bNtpserver = true;
    }
    savecfg();
    startNtpServer();
}

void QIperfd::startNtpServer()
{
    if (bNtpserver){
        if (m_ntpserver){
            onLog("stop old Ntp Server");
            m_ntpserver->disconnect();
            delete(m_ntpserver);
        }
        onLog("start Ntp Server");
        m_ntpserver = new NtpServer(this);
    }else{
        onLog("Ntp Server not enable!!");
        if (m_ntpserver){
            onLog("stop Ntp Server");
            m_ntpserver->disconnect();
            delete(m_ntpserver);
        }
    }
}

void QIperfd::initiperf2(QString tmp, QString tmp_path, QString arch)
{
#if defined(Q_OS_WIN32)
    // windows, iperf files
    m_iperfexe20 = apppath + QDir::separator() + "windows" +QDir::separator() + "x86"+QDir::separator() + "iperf2.exe";
    m_iperfexe20 = QDir::toNativeSeparators(m_iperfexe20);
#else

    m_iperfexe20 = tmp + tmp_path + QDir::separator() + "iperf2";
    if (QFileInfo::exists(m_iperfexe20))
    {
        QFile::remove(m_iperfexe20);
    }
    // iperf2
    #if defined(Q_OS_ANDROID)
        QFile i2File(":/android/" + arch + "/iperf");
    #else
        Q_UNUSED(arch)
        QFile i2File(apppath+QDir::separator()+"linux"+QDir::separator()+"iperf2");
        if (i2File.exists()) {
            // make file execuable
            if (i2File.copy(m_iperfexe20)){
                QFile iperf2File(m_iperfexe20);
                iperf2File.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                          QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup |
                                          QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther);
            }else{
                debug("copy file to " + m_iperfexe20 + " fail", 5);
            }
        }else{
            debug(i2File.fileName() + " NOT EXIST!!", 5);
        }
    #endif
#endif
}

void QIperfd::initiperf21(QString tmp, QString tmp_path, QString arch)
{
#if defined(Q_OS_WIN32)
    // windows, iperf files
    m_iperfexe21 = apppath + QDir::separator() + "windows"+ QDir::separator() + arch + QDir::separator() + "iperf2.1.exe";
    m_iperfexe21 = QDir::toNativeSeparators(m_iperfexe21);
#else
    m_iperfexe21 = tmp + tmp_path + QDir::separator() + "iperf2.1";
    if (QFileInfo::exists(m_iperfexe21))
    {
        QFile::remove(m_iperfexe21);
    }
    // iperf2.1
    #if defined(Q_OS_ANDROID)
        QFile i21File(":/android/" + arch + "/iperf");
    #else
        Q_UNUSED(arch)
        QFile i21File(apppath+QDir::separator()+"linux"+QDir::separator()+"iperf2.1");
        if (i21File.exists()) {
            // make file execuable
            if (i21File.copy(m_iperfexe21)){
                QFile iperf21File(m_iperfexe21);
                iperf21File.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                           QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup |
                                           QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther);
            }else{
                debug("copy file to " + m_iperfexe21 + " fail", 5);
            }
        }else{
            debug(i21File.fileName() + " NOT EXIST!!", 5);
        }
    #endif
#endif
}
void QIperfd::initiperf22(QString tmp, QString tmp_path, QString arch)
{
#if defined(Q_OS_WIN32)
    // windows, iperf files
    m_iperfexe22 = apppath + QDir::separator() + "windows"+ QDir::separator() + arch +QDir::separator() + "iperf2.2.n.exe";
    m_iperfexe22 = QDir::toNativeSeparators(m_iperfexe22);
#else
    m_iperfexe22 = tmp + tmp_path + QDir::separator() + "iperf2.2.n";
    if (QFileInfo::exists(m_iperfexe22))
    {
        QFile::remove(m_iperfexe22);
    }
    // iperf2.2
    #if defined(Q_OS_ANDROID)
        QFile i22File(":/android/" + arch + "/iperf");
    #else
        Q_UNUSED(arch)
        QFile i22File(apppath+QDir::separator()+"linux"+QDir::separator()+"iperf2.2.n");
        if (i22File.exists()) {
            // make file execuable
            if (i22File.copy(m_iperfexe22)){
                QFile iperf22File(m_iperfexe22);
                iperf22File.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                           QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup |
                                           QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther);
            }else{
                debug("copy file to " + m_iperfexe22 + " fail", 5);
            }
        }else{
            debug(i22File.fileName() + " NOT EXIST!!", 5);
        }
    #endif
#endif
}
void QIperfd::initiperf3(QString tmp, QString tmp_path, QString arch)
{
#if defined(Q_OS_WIN32)
    // windows, iperf files
    m_iperfexe3 = apppath + QDir::separator() + "windows" +QDir::separator() + arch + QDir::separator() + "iperf3.exe";
    m_iperfexe3 = QDir::toNativeSeparators(m_iperfexe3);
#else
    if (bUseSysIperf){
        m_iperfexe3 = "/usr/bin/iperf3";
    }else{
        m_iperfexe3 = tmp + tmp_path + QDir::separator() + "iperf3";
        if (QFileInfo::exists(m_iperfexe3))
        {
            QFile::remove(m_iperfexe3);
        }
// iperf3
#if defined(Q_OS_ANDROID)
        QFile i3File(":/android/" + arch + "/iperf3");
#else
        Q_UNUSED(arch)
        QFile i3File(apppath+QDir::separator()+"linux"+QDir::separator()+"iperf3");
        if(i3File.exists()){
            if (i3File.copy(m_iperfexe3)){
                QFile iperf3File(m_iperfexe3);
                iperf3File.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                          QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup |
                                          QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther);
            }else{
                debug("copy file to " + m_iperfexe3 + " fail");
            }
        }else{
            debug(i3File.fileName() + " NOT EXIST!!");
        }
#endif
    }
#endif
}

void QIperfd::initIperf(QString apppath)
{
    Q_UNUSED(apppath)
    // iperf control interface, accept add/del iperf setting from remote
    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString tmp_path = "";
    QString arch = "";
#if defined(Q_OS_ANDROID) || defined(Q_OS_WIN32)
    arch = QSysInfo::buildCpuArchitecture();
#endif
#if defined(Q_OS_LINUX)
    // linux/android path
    #if !defined(Q_OS_ANDROID)
        tmp_path = QString(QDir::separator()) + "qiperf";
        QDir dir(tmp + tmp_path);
        if (!dir.exists())
        {
            dir.mkdir(tmp + tmp_path);
        }
    #else
        QString tmp_path = "";
    #endif

#else
    debug(" Not Support platform!!", 5);

#endif \
    //TODO: check we have newer version of iperf, remove old !!
    initiperf2(tmp, tmp_path, arch);
    initiperf21(tmp, tmp_path, arch);
    initiperf22(tmp, tmp_path, arch);
    if (bUseSysIperf){
        m_iperfexe2 ="/usr/bin/iperf";
    }else{
        m_iperfexe2 = m_iperfexe22;
    }
    initiperf3(tmp, tmp_path, arch);

}

void QIperfd::getIperfVer(QString cmd, double ver)
{

    QProcess process;
    QStringList args;
    args.append("-v");
    // process.startDetached(m_iperfexe3, args);
#if QT_VERSION < 0x060000  // < 6.0
    process.start(cmd, args);
#else
#if defined(Q_OS_WIN32)
    QString c = "\"" + cmd + "\"" + " -v";
#else
    QString c = cmd + " -v";
#endif
    process.startCommand(c); //Qt6.0
#endif
    if (!process.waitForFinished(5000)){//wait 5 sec
        debug("Error run cmd: " + cmd + " " + args.join(" ") + " Fail");
        debug(QString("(%1)").arg(QString::fromUtf8(process.readAll())));
    }

    QString out;
    if ((ver == 2.0)||(ver == 2.1)||(ver == 2.2)){
        out = process.readAllStandardError();
        out = out + process.readAllStandardOutput();
    }else{
        out = process.readAllStandardOutput();
    }
    QStringList ds = out.split("\n");
    QString extra="";
    foreach (QString line, ds) {
        if (line.startsWith("iperf")){
            QStringList tmps =line.split(" ");
            if (tmps.length()>=2){
                if (tmps.length()>=7){
                    extra = QString("%1 %2 %3").arg(tmps[3], tmps[4], tmps[5]);
                }
                if (ver == 2.0){
                    m_iperfexe20ver = tmps[2] + " "+ extra;
                }
                if (ver == 2.1){
                    m_iperfexe21ver = tmps[2] + " "+ extra;
                }
                if (ver == 2.2){
                    m_iperfexe22ver = tmps[2] + " "+ extra;
                }
                if (ver == 3.0){
                    m_iperfexe3ver = tmps[1];
                }
                break;
            }
        }
    }
}

void QIperfd::initJIOOpenWRT()
{
    if (bIsJIOOpenWRT){
        //
        QFile fJio(":/jio/jiocmd");
        if (fJio.open(QIODevice::ReadOnly)) {
            //basic commands
            QByteArray jsonData = fJio.readAll();
            fJio.close();

            QJsonParseError parseError;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                qDebug() << "Failed to parse JSON:" << parseError.errorString();
                return; // Or handle the error appropriately
            }
            jiocmdObj = jsonDoc.object();
            // rootObject.keys()
            jiocmdRespObj = jiocmdObj.value("RESPONSE").toObject();
            // qDebug() << "jiocmdRespObj:" << jiocmdRespObj.toVariantMap();
        }else {
            qDebug() << "Failed to open " << fJio.fileName() << " for reading:" << fJio.errorString();
        }

        QString scmd="";
        if (bIsAM7){
            //AP
            scmd = JIO_SET_AM7_IPERF_BETTER;
        }else{
            //STA
            scmd = JIO_SET_CM7_IPERF_BETTER;
        }
        QProcess process;
        onLog("Exec cmd: " + scmd);
#if QT_VERSION < 0x060000  // < 6.0
        QStringList cmds = scmd.split(" ");
        process.start(cmds[0], cmds.mid(1, cmds.count()-1));
#else
        process.startCommand(scmd);
#endif
        process.waitForFinished(3000);// Optional: blocks until done

        QString output = process.readAllStandardOutput();
        QString error = process.readAllStandardError();
        if (!output.isEmpty()){
            onLog("cmd STD Output:" + output);
        }
        if (!error.isEmpty()){
            onLog("cmd STD Error:" + error);
        }
    }
}

void QIperfd::runRequest(QString refid, QString from, QString reqcmd, QString cmds)
{
    qDebug() << "runRequest: refid:" << refid
             << " from: " << from
             << " reqcmd:" << reqcmd
             << " exec cmd:" << cmds;
    QString rpcmd="";
    if (jiocmdRespObj.contains(reqcmd)){
        rpcmd = jiocmdRespObj.value(reqcmd).toString();
        qDebug() << " response tag:" << rpcmd;
    }else {
        qDebug() << " No supported reqcmd:" << reqcmd;
        return;
    }

    QProcess process;
    // TODO: other platform
#if QT_VERSION < 0x060000  // < 6.0
    process.start("bash", QStringList() << cmds);
#else
    process.startCommand(cmds);
#endif
    if (!rpcmd.isEmpty()){
        process.waitForFinished();
        QString output = process.readAllStandardOutput();
        // qDebug() << "output:" << output;
        QString rs = parserResponse(rpcmd, output);
        // qDebug() << "Response:" << rs;
        // QString erroutput = process.readAllStandardError();
        // qDebug() << "erroutput:" << erroutput;
        QString res = QString("%1:%2:%3:%4").arg(CMD_REQUEST_RESULT, refid, rpcmd, rs);
        qDebug() << "Response res:" << res;
        int rc = m_wsserver->sendTextMessage(res, from);
        if (rc<=0){
            debug(QString("error send %1").arg(res) , 2);
        }
    }else {
        debug(QString("No RESPONSE data of %1").arg(reqcmd) , 2);
    }
}

QString QIperfd::parserResponse(QString rpcmd, QString data)
{
    QString rs="";
    QString reg="";
    QString pattern="";
    QJsonObject rObj;
    //parse data
    if (jiocmdObj.contains(rpcmd)){
        QJsonObject pasObj = jiocmdObj.value(rpcmd).toObject();
        foreach(QString line, data.split("\n")){
            foreach(QString key, pasObj.keys()){
                if (line.contains(key)){
                    reg = pasObj.value(key).toString();
                    pattern = QString("%1%2").arg(QRegularExpression::escape(key), reg);
                    QRegularExpression regex(pattern);
                    QRegularExpressionMatch match = regex.match(line);
                    if (match.hasMatch()) {
                        QString capStr = match.captured(1);
                        rObj.insert(key, capStr.toDouble());
                    }
                }
            }
        }
        // qDebug() << "rObj:" << rObj;
        QJsonDocument doc(rObj);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        qDebug() << "strJson: " << strJson ;
        rs = strJson;
    }else{
        debug(QString("No data of %1").arg(rpcmd) , 2);
    }
    return rs;
}
