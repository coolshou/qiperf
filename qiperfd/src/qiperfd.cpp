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

#include "qiperfd.h"
#include "../src/comm.h"
#include "../src/versions.h"
#if defined(Q_OS_WIN32)
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#endif
#include <QDebug>

QIperfd::QIperfd(PipeServer *pserver, QObject *parent)
    : QObject{parent}, m_pserver(pserver), m_fileclient(nullptr)
{
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
    qDebug() << "Max socket buffer sizes: " << buffsize << " K";
    qDebug() << "TODO: set Max socket buffer sizes to ?";
    // m_myinfo->setSysBufferSize(4*static_cast<uint>(BUFFER_SIZES::MB));
    // notice qiperfc info
    m_udpsrv = new UdpSrv(QIPERFD_BPORT, getManagerInterface(), m_myinfo);
    connect(this, &QIperfd::setMgrIfname, m_udpsrv, &UdpSrv::setIfname);
    // qDebug() << "INFO: " << info;
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
    // m_ntpsync = new NtpSync(this);
    // m_ntpsync->sync("192.168.70.147");

    // system service manager
    qiperfdlog = tmppath+QIPERFD_NAME+".log";
    qInfo() << "FileWatcher: " << QDir::toNativeSeparators(qiperfdlog);
    m_filewatcher = new FileWatcher(qiperfdlog);
    connect(m_filewatcher, &FileWatcher::onNewLine, this, &QIperfd::onNewLine);

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
    qInfo() << qApp->applicationPid() <<"," << text;
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
}

void QIperfd::savecfg()
{
    cfg->beginGroup("manager");
    cfg->setValue("ifname", mgr_ifname);
    cfg->setValue("port", mgr_port);
    cfg->setValue("EnableNtpServer", bNtpserver);
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

int QIperfd::add(QString refrow, int version, QString m_cmd, QString args, uint port,
                 QString bndaddr, QString target,
                 QString parallel, QString protocal, bool bidir, bool reverse,
                 int interval, int delaytime)
{ // add a IperfWorker to run iperf server/client
    // TODO: check host/port used?
    QThread *iperf_th = new QThread();
    int idx = m_threads.count();
    m_threads.insert(idx, iperf_th);
    //    m_threads.append(iperf_th);
    //    int idx = m_threads.count()-1;
    IperfWorker *iperfer = new IperfWorker(idx, version, m_cmd, args, port,
                                           bndaddr, target, bidir, reverse,
                                           interval, delaytime);
    iperfer->setRefRow(refrow);
    iperfer->setExtra(parallel, protocal, port);
//    connect(iperfer, &IperfWorker::onStdout, this, &QIperfd::readStdOut);
    connect(iperfer, &IperfWorker::onStderr, this, &QIperfd::onErrored);
    connect(iperfer, &IperfWorker::log, this, &QIperfd::onIperfLog);
    connect(iperfer, &IperfWorker::started, this, &QIperfd::onStarted);
    connect(iperfer, &IperfWorker::finished, this, &QIperfd::onFinished);
    // connect(iperfer, &IperfWorker::finished, iperfer, &IperfWorker::deleteLater);
    connect(iperfer, &IperfWorker::onThroughput, this, &QIperfd::onThroughput);

    connect(iperf_th, &QThread::started, iperfer, &IperfWorker::work);
    connect(iperf_th, &QThread::finished, iperf_th, &QThread::deleteLater);
    connect(iperf_th, &QThread::finished, iperfer, &IperfWorker::deleteLater);
    iperfer->moveToThread(iperf_th);
    connect(this, &QIperfd::setStop, iperfer, &IperfWorker::setStop);

    //    m_iperfworkers.append(iperfer);
    m_iperfworkers.insert(idx, iperfer);
    return idx;
}

int QIperfd::add(QString refrow, QVariantMap jsondata)
{
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
        qDebug() << "Not support Iperf version:" << ver;
        return -1;
    }
    uint port = jsondata["port"].toUInt();
    QString binaddr = jsondata["bind"].toString();
    QString target = jsondata["target"].toString();
    QString parallel = jsondata["parallel"].toString(); // for server mode use
    QString protocal = jsondata["protocal"].toString(); // for server mode use
    bool bidir = jsondata["bidir"].toBool(); // for server mode use
    bool reverse = jsondata["reverse"].toBool(); // for server mode use
    int interval = jsondata["interval"].toInt();
    int delaytime = jsondata["delaytime"].toInt();

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
        qDebug() << "Not support Iperf version:" << ver;
        return -1;
    }
    return add(refrow, ver, cmd, args, port, binaddr,
               target, parallel, protocal, bidir, reverse, interval, delaytime);
}

void QIperfd::del(int idx)
{
    //delete specify m_iperfworkers & m_threads
    //
    if (m_threads.contains(idx))
    {
        // qDebug() << "QIperfd::del m_threads:" << idx;
        m_threads.remove(idx);
    }
    if (m_iperfworkers.contains(idx))
    {
        // qDebug() << "remove m_iperfworkers:" << idx;
        m_iperfworkers.remove(idx);
    }

    m_runstatus[idx]=0;
}

int QIperfd::addIperfServer(QString refrow, int version, uint port, QString bindHost)
{
    qDebug() << "addIperfServer:" << bindHost << ":" << port ;

    QString cmd;
    QString argbind;
    // add a iperf server
    if (version==static_cast<int>(IPERF_VER::V3)){
        cmd = m_iperfexe3;
        argbind = " --bind ";
    }else if ((version==static_cast<int>(IPERF_VER::V2))||
               (version==static_cast<int>(IPERF_VER::V21))||
               (version==static_cast<int>(IPERF_VER::V22))
               ){
        cmd = m_iperfexe2;
        argbind = " -B ";
    }else{
        qDebug() << "Not support Iperf version:" << version ;
        return -1;
    }
    QString args=" -s ";
    if (!(bindHost=="")) {
        args.append(argbind);
        args.append(bindHost);
    }
    return add(refrow, version, cmd, args, port, bindHost);
}

int QIperfd::addIperfClient(QString refrow, int version, uint port, QString Host, QString iperfargs)
{
    qDebug() << "addIperfClient:" << Host << ":" << port ;
    QString cmd;
    // add a iperf server
    if (version==static_cast<int>(IPERF_VER::V3)){
        cmd = m_iperfexe3;
    }else if ((version==static_cast<int>(IPERF_VER::V2))||
               (version==static_cast<int>(IPERF_VER::V21))||
               (version==static_cast<int>(IPERF_VER::V22))
               ){
        cmd = m_iperfexe2;
    }else{
        qDebug() << "Not support Iperf version:" << version ;
        return -1;
    }
    QString args = iperfargs;
    return add(refrow, version, cmd, args, port);
}

void QIperfd::start(int idx)
{
    try{
        QThread *th = m_threads.value(idx);
        m_iperfworkers.value(idx)->setIperfLogPath(tmpfilepath+QDir::separator()+s_starttime);
        th->start();
    }catch (const std::exception &e) {
        // Handle the exception and show an error message
        qDebug() << "Exception Caught" << e.what();
    }
}
void QIperfd::startAll()
{
    QString tmp = tmpfilepath+QDir::separator()+s_starttime;
    QDir d(tmp);
    if (!d.exists()){
        d.mkpath(tmp);
    }
    // start all thread
    for (auto it = m_threads.begin(); it != m_threads.end(); ++it)
    {
        start(it.key());
        QCoreApplication::processEvents(QEventLoop::AllEvents);
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
    // stop all iperfworker
    // for (auto it = m_iperfworkers.begin(); it != m_iperfworkers.end(); ++it)
    // {
    //     it.value()->setStop();
    // }
    emit setStop();
}

void QIperfd::clear()
{
    //clear all m_iperfworkers & m_threads
    if (!m_iperfworkers.isEmpty()){
        for (auto it = m_iperfworkers.begin(); it != m_iperfworkers.end();) {
            it = m_iperfworkers.erase(it);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }
    if (!m_threads.isEmpty()){
        for (auto it = m_threads.begin(); it != m_threads.end();) {
            it = m_threads.erase(it);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }
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
        qDebug() << "CMD_IFNAMES:" << netObjs;
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
        qDebug() << "CMD_QIPERFD_RESTART: " << msg;
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
        qDebug() << "CMD_NTP_START:ntpmode:" << ntpmode;
        setNtpServer(ntpmode);
    }
    else
    {
        qDebug() << "handle json: " << msg ;
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
                // TODO: refrow
                add("0", ver, cmd, args, port);
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
    qDebug() << "onStarted: " << msg;
    m_wsserver->sendTextResult(msg);
    m_runstatus[m_idx]=1;
}

void QIperfd::onFinished(int idx, int exitCode, int exitStatus, QString ipport, QString filename)
{
    QString msg = QString(CMD_IPERF_STOPED)+":"+ QString::number(idx);
    msg = msg + ":" + QString::number(exitCode)+  ":" + QString::number(exitStatus);
    msg = msg + ":" + ipport;
    onLog("onFinished: " + msg);
    m_wsserver->sendTextResult(msg);
    if (!filename.isEmpty()){
        if(QFileInfo::exists(filename)){
            // qDebug() << "enqueueFile: " << filename;
            m_fileclient->enqueueFile(filename);
        }else{
            onLog("file to send not Exist: " + filename);
        }
    }
    del(idx);

}

void QIperfd::onThroughput(int idx, QString sInterval, QString data)
{
    QString s = QString(CMD_IPERF_TP_DATA)+":"+ QString::number(idx)+":"+
                sInterval+":"+ data;
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
    m_pserver->sendMessage(QIPERFDLOG+QString("：")+line);
}

void QIperfd::doRestartQIperfd()
{
    qDebug() << "doRestartQIperfd";
#if defined(Q_OS_LINUX)
#if !defined(Q_OS_ANDROID)
    // cmd = "systemctl restart qiperfd";
    const QStringList arguments ={"restart", "qiperfd"};
    qApp->quit();
    QProcess::startDetached("systemctl", arguments);
#else
    qDebug() << "TODO: restart android qiperfd service"
#endif
#elif defined(Q_OS_WINDOWS)
    QString taskName = "startqiperfd";
    // Set task to run 1 sec from now
    QDateTime runTime = QDateTime::currentDateTime().addSecs(1);
    //nssm.exe restart "qiperfd"
    QString taskCommand = "\"" + m_nssm + "\" restart qiperfd ";
    if (createScheduledTask(taskName, taskCommand, runTime)) {
        qDebug() << "Task created successfully.";
    } else {
        qDebug() << "Failed to create task.";
    }
    // if (createSchedule("startqiperfd", cmd, 5)){
    //     qApp->quit();
    // }else{
    //     qDebug() << "createSchedule Fail";
    // }
#else
    qDebug() << "do Restart QIperfd for system :" << QSysInfo::productType();
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
    // qDebug() << "onSSHTaskStarted:" << QString::number(port);
    informMessage(QString("%1:%2:%3").arg(CMD_SSH_OPENED, idx, QString::number(port)));
}

void QIperfd::onSSHTaskError(QString idx, QString errormsg)
{
    qDebug() << idx <<" onSSHTaskError:" << errormsg;
    informMessage(QString("%1:%2:%3").arg(CMD_SSH_FAIL, idx, errormsg));
}

#if defined(Q_OS_WINDOWS)
bool QIperfd::createSchedule(QString name, QString cmd, int idelay)
{
    // create a windows Schedule task
    QString program = "schtasks";
    // Get the current time and add one second
    QDateTime currentTime = QDateTime::currentDateTime().addSecs(idelay);
    QString startTime = currentTime.toString("HH:mm:ss");

    QStringList arguments;
    arguments << " /Create "
              << " /SC " << "ONCE"
              << " /TN " << name
              << " /TR " << cmd
              << " /ST " << startTime
              << " /F ";
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    qDebug() << "createSchedule cmd: " << program << " " << arguments;
    process.startDetached(program, arguments);
    if (!process.waitForStarted(5000)){
        qDebug() << "start schtasks '" << program << " " << arguments.join(" ") << "' Fail or timeout" << Qt::endl
                 << "(" << process.readAll() << ")";
        return false;
    }else{
        return true;
    }

}
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

bool QIperfd::createScheduledTask(const QString &taskName, const QString &taskCommand, const QDateTime &runTime) {
    HRESULT hr = S_OK;

    // 1. Initialize COM
    // CoInitializeEx is safe to call multiple times for the same thread.
    // It's good practice to balance with CoUninitialize if you manage COM lifespan within a single function.
    // However, for a GUI app, it's often initialized once at app start.
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // Or COINIT_MULTITHREADED
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        qDebug() << QString("CoInitializeEx failed: %1").arg(comErrorToString(hr));
        return false;
    }

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
        hr = pRegInfo->put_Description(toBSTR(QString("Task created by Qt application API for testing '%1'").arg(taskCommand)));
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
        hr = pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN); // or TASK_LOGON_GROUP for System account
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
            _variant_t(),         // User (omit for current user or if set in principal)
            password,             // Password (omit for current user, INTERACTIVE_TOKEN, or SYSTEM)
            TASK_LOGON_INTERACTIVE_TOKEN, // Logon type
            _variant_t(),         // SDDL (Security Descriptor Definition Language)
            &pRegisteredTask      // Output: Registered task object
            );
        if (FAILED(hr)) _com_issue_error(hr);

        qDebug() << QString("Task '%1' registered successfully.").arg(taskName);
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
        qDebug() << QString("COM Error during task creation: %1 (HRESULT: 0x%2)")
                       .arg(comErrorToString(error.Error()))
                       .arg(error.Error(), 8, 16, QChar('0').toUpper());
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
        qDebug() << "An unknown error occurred during task creation.";
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
    //    finally {
    //     // CoUninitialize is generally only called once at app shutdown for main thread.
    //     // If COM is initialized/uninitialized per-function, ensure it's balanced.
    //     // CoUninitialize();
    // }
}

bool QIperfd::runScheduledTask(const QString &taskName) {
    HRESULT hr = S_OK;
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        qDebug() << QString("CoInitializeEx failed: %1").arg(comErrorToString(hr));
        return false;
    }

    ITaskService* pService = nullptr;
    ITaskFolder* pRootFolder = nullptr;
    IRegisteredTask* pRegisteredTask = nullptr;

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

        _variant_t params = _variant_t(); // No parameters for the run
        hr = pRegisteredTask->Run(params);
        if (FAILED(hr)) _com_issue_error(hr);

        return true;

    } catch (const _com_error& error) {
        qDebug() << QString("COM Error during task run: %1 (HRESULT: 0x%2)")
                       .arg(comErrorToString(error.Error()))
                       .arg(error.Error(), 8, 16, QChar('0').toUpper());
        return false;
    } catch (...) {
        qDebug() << "An unknown error occurred during task run.";
        return false;
    }
}

bool QIperfd::deleteScheduledTask(const QString &taskName) {
    HRESULT hr = S_OK;
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        qDebug() << QString("CoInitializeEx failed: %1").arg(comErrorToString(hr));
        return false;
    }

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
            qDebug() << QString("Task '%1' not found, nothing to delete.").arg(taskName);
            return true; // Consider it successful deletion if it wasn't there
        }
        qDebug() << QString("COM Error during task deletion: %1 (HRESULT: 0x%2)")
                       .arg(comErrorToString(error.Error()))
                       .arg(error.Error(), 8, 16, QChar('0').toUpper());
        return false;
    } catch (...) {
        qDebug() << "An unknown error occurred during task deletion.";
        return false;
    }
}

#endif
void QIperfd::onWSactMessage(QString msg)
{
    //handle act message from websocket
    long long cut = msg.indexOf(':', 0);
    QString act = msg.left(cut);
    qDebug()<< "onWSactMessage: " << act;
    msg = msg.right(msg.length()-cut-1);
    // expect in json format
    if (act.startsWith(CMD_IPERF_ADD)){
        QJsonParseError error;
        cut = msg.indexOf(':', 0);
        QString refrow = msg.left(cut);
        msg = msg.right(msg.length()-cut-1);
        qDebug()<< "CMD_IPERF_ADD: " << msg;
        QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError){
            add(refrow, doc.toVariant().toMap());
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
        qInfo() << "SET to Report throughput data: " << msg;
    }else if (act.startsWith(CMD_IPERF_UNREG)){
        QStringList d = msg.split(":");
        QString starttime = d[0];
        QString bindkey = d[1];
        // bReportTPData = false;
        m_directions[starttime][bindkey].clear();
        qInfo() << "SET to NOT Report throughput data: " << msg;
    }else if (act.startsWith(CMD_IPERF_CLEAR)){
        clear();
    }else if (act.startsWith(CMD_IPERF_START)){
        s_starttime = msg;
        m_starttime =QDateTime::fromString(s_starttime, DATETIME_NOW_FORMAT);
        qInfo() << "s_starttime: " << s_starttime;
        startAll();
    }else if (act.startsWith(CMD_IPERF_STOP)){
        stopAll();
    }else if (act.startsWith(CMD_PING)){
        qDebug()<< "CMD_PING";
        cut = msg.indexOf(':', 0);
        QString refrow = msg.left(cut);
        msg = msg.right(msg.length()-cut-1);
        m_icmpping = new IcmpPing(refrow, msg, nullptr);
    }else if (act.startsWith(CMD_NTP_SYNC)){
        qDebug()<< "CMD_NTP_SYNC";
        cut = msg.indexOf(':', 0);
        msg = msg.right(msg.length()-cut-1);
        //TODO: do ntp sync, m_ntpsync->sync(msg);
    }else if (act.startsWith(CMD_SERIAL_ADD)){
        // TODO: create serial and bind to TCP server
        // idx:comport:BaudRate:DataBits:Parity:StopBits:FlowControl
        qInfo() << "CMD_SERIAL_ADD: " << msg;
        QStringList d = msg.split(":");
        if (d.length()==7){
            long long port = 0;
            SerialTask *task = nullptr;
            QString idx = d[0];
            QString comport = d[1];
            QString baudrate = d[2];
            // qDebug() << "m_serialtasks: " << m_serialtasks << "  comport: " << comport;
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
                    qDebug() << comport << " exist!! Using port:" << QString::number(localport);
                    // TODO: update setting?
                    task->setConfig(QString::number(localport), baudrate,
                                    databits, parity, stopbits, flowcontrol);
                    task->close();
                    QTimer::singleShot(0, task, SLOT(init())); // start it
                    onSerialTaskStarted(idx, localport);
                }else{
                    QString emsg = task->getLastError();
                    qDebug() << "SerialTask is not running: " << emsg;
                    onSerialTaskError(idx, emsg);
                }
            }
        }else{
            qDebug() << " Wrong format of create serial: " << msg;
        }
    }else if (act.startsWith(CMD_SERIAL_DEL)){
        //
        // qInfo() << "CMD_SERIAL_DEL: " << msg;
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
                qDebug() << comport << " does not in m_serialtasks!! \n" << m_serialtasks;
                informMessage(QString("%1:%2 %3").arg(CMD_SERIAL_FAIL, comport, "Not Exist"));
            }
        }else{
            qDebug() << " Wrong format of delete serial: " << msg;
        }
    }else if (act.startsWith(CMD_SSH_ADD)){
        // qInfo() << "CMD_SSH_ADD: " << msg;
        QStringList d = msg.split(":");
        if (d.length()==7){
            long long port = 0;
            SSHTask *task = nullptr;
            try{
                QString idx = d[0];
                QString sshTarget = d[1];
                QString sshPort = d[2];
                QString key = sshTarget + ":" + sshPort;
                QString username = d[3];
                QString password = d[4];
                QString privateKeyFile = d[5];
                int timeout = d[6].toInt();
                // qDebug() << "idx:" << idx << " sshTarget:" << sshTarget
                //          << " sshPort:" << sshPort
                //          << " username:" << username
                //          << " password:" << password
                //          << " privateKeyFile:" <<  privateKeyFile
                //          << " timeout:" <<  QString::number(timeout);
                //TODO: m_sshtasks's key format?
                if (!m_sshtasks.contains(key)){ // not exist
                    port =  QIPERF_SSHPORT + m_sshtasks.count();
                    task = new SSHTask(idx, sshTarget, sshPort,
                                       "any", QString::number(port),
                                       VirtualDeviceTcp::Mode::BINARY,
                                       username, password, privateKeyFile, timeout);
                    connect(task, &SSHTask::finished, this, &QIperfd::onSSHTaskFinished);
                    connect(task, &SSHTask::started,  this, &QIperfd::onSSHTaskStarted);
                    m_sshtasks.insert(key, task);
                    QTimer::singleShot(0, task, SLOT(init())); // start it
                } else {
                    task = m_sshtasks.value(key);
                    quint16 localport = task->getLocalPort();
                    if (task->isRunning()){
                        qDebug() << sshTarget << " exist!! Using port:" << QString::number(localport);
                        // TODO: update setting?
                        // task->setConfig(QString::number(localport), baudrate,
                        //                 databits, parity, stopbits, flowcontrol);
                        // qDebug() << "SSHTask close";
                        task->close();
                        // qDebug() << "reinit SSHTask";
                        QTimer::singleShot(0, task, SLOT(init())); // start it
                        onSSHTaskStarted(idx, localport);
                    }else{
                        QString emsg = "SSHTask is not running: " + task->getLastError();
                        qDebug() << emsg;
                        onSSHTaskError(idx, emsg);
                    }
                }
            } catch (const std::exception &e) {
                qDebug() << "SSHTask error: " << e.what();
            }
        }else{
            qDebug() << " Wrong format of create ssh: " << msg;
            // onSSHTaskError(idx, QString("Wrong format of create ssh: %1").arg(msg));
        }
    }else if (act.startsWith(CMD_SSH_DEL)){
        // msg format: "127.0.0.1:192.168.0.90:22"
        qInfo() << "CMD_SSH_DEL: " << msg;
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
                qDebug() << key << " does not in m_sshtasks!! \n" << m_sshtasks;
                // onSSHTaskError(idx, QString("%1:%2").arg(target, "Not Exist"))
            }
        }else{
            qDebug() << " Wrong format of delete ssh: " << msg;
        }
    }else {
        qDebug() << " Unknown action:" << act  << " \n==========\n" << msg;
        qDebug() << "\n==========";
    }
}

void QIperfd::onNewClient(QHostAddress addr)
{
    onNewLine("onNewClient: " +addr.toString());
    qDebug() << "onNewClient: "  << addr.toString();
    if (m_fileclient){
        qDebug() << "m_fileclient exist: " << m_fileclient->getTargetAddress() << " new: " << addr.toString();
    }
    //TODO: multi file client
    m_fileclient = new FileClient(QIPERF_FILEPORT, addr.toString());
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
    // Initialize COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        // QMessageBox::critical(this, "Error", "Failed to initialize COM library.");
        err = "Error: Failed to initialize COM library.";
        informMessage(err, true);
        return -1;
    }

    // Initialize security
    hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities
        NULL                         // Reserved
        );

    if (FAILED(hres)) {
        // QMessageBox::critical(this, "Error", "Failed to initialize security.");
        err = "Error: Failed to initialize security.";
        informMessage(err, true);
        CoUninitialize();
        return -1;
    }

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
        CoUninitialize();
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
        CoUninitialize();
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
        CoUninitialize();
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
        CoUninitialize();
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
    CoUninitialize();
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
        qDebug() << "Error run cmd: " << command << " Fail";
        qDebug() << "(" << process.readAll() << ")";
        return -1;
    }

    // Read the output
    QByteArray output = process.readAllStandardOutput();
    QByteArray errorOutput = process.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        qDebug() << "checkFirewallStatus Error:\n" << errorOutput << "\n\n CMD:" << command << "\n";
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
            qInfo() << "stop old Ntp Server";
            m_ntpserver->disconnect();
            delete(m_ntpserver);
        }
        qInfo() << "start Ntp Server";
        m_ntpserver = new NtpServer(this);
    }else{
        qInfo() << "Ntp Server not enable!!";
        if (m_ntpserver){
            qInfo() << "stop Ntp Server";
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
                qDebug() << "copy file " << " to " << m_iperfexe20 << " fail";
            }
        }else{
            qDebug() << i2File.fileName() << " NOT EXIST!!";
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
                qDebug() << "copy file " << " to " << m_iperfexe21 << " fail";
            }
        }else{
            qDebug() << i21File.fileName() << " NOT EXIST!!";
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
    // iperf2.1
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
                qDebug() << "copy file " << " to " << m_iperfexe22 << " fail";
            }
        }else{
            qDebug() << i22File.fileName() << " NOT EXIST!!";
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
                qDebug() << "copy file " << " to " << m_iperfexe3 << " fail";
            }
        }else{
            qDebug() << i3File.fileName() << " NOT EXIST!!";
        }
    #endif
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
    qDebug() << " Not Support platform!!";

#endif \
    //TODO: check we have newer version of iperf, remove old !!
    initiperf2(tmp, tmp_path, arch);
    initiperf21(tmp, tmp_path, arch);
    initiperf22(tmp, tmp_path, arch);
    m_iperfexe2 = m_iperfexe22;
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
    qDebug() << "Qt6 getIperfVer: " << c << " arg:" << args;

    process.startCommand(c); //Qt6.0
    // process.setProgram(cmd);
    // process.setArguments(args);
    // process.start();
#endif
    if (!process.waitForFinished(5000)){//wait 5 sec
        qDebug() << "Error run cmd: " << cmd << " " << args.join(" ") << " Fail";
        qDebug() << "(" << process.readAll() << ")";
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
