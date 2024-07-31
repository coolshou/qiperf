#include <QCoreApplication>
#include <QStandardPaths>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QNetworkInterface>
#include <QDir>
#include <QEventLoop>

#include "qiperfd.h"
#include "../src/comm.h"
#include "../src/versions.h"

#include <QDebug>

QIperfd::QIperfd(PipeServer *pserver, QObject *parent)
    : QObject{parent}, m_fileclient(nullptr)
{
    bReportTPData = false;
    onLog(QString(QIPERFD_NAME) + ":" + QIPERFD_VERSION);
    tmpfilepath =  QStandardPaths::writableLocation(QStandardPaths::TempLocation)+"/"+ QIPERF_NAME + "/data";
    QDir d(tmpfilepath);
    if (!d.exists()){
        if(!d.mkpath(tmpfilepath)){
            onLog("ERROR: mkdir "+ tmpfilepath+ " Fail");
        }
    }
    onLog("tmpfilepath: "+ tmpfilepath);
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
    QString apppath = qApp->applicationDirPath(); // app run time path:/home/coolshou/sdb/download/work/qiperf/Debug
    loadcfg(apppath);
    //    qDebug() << "start UdpSrv" << Qt::endl;
    //
    m_iperfwrapper = new IperfWrapper(this);
//    m_myinfo = new MyInfo(getManagerInterface());
    m_myinfo = new MyInfo(mgr_ifname);
    connect(this, &QIperfd::setMgrIfname, m_myinfo, &MyInfo::setIfname);
    QString info = m_myinfo->collectInfo();
    // notice qiperfc info
    m_udpsrv = new UdpSrv(QIPERFD_BPORT, getManagerInterface(), m_myinfo);
    connect(this, &QIperfd::setMgrIfname, m_udpsrv, &UdpSrv::setIfname);
    m_udpsrv->setSendMsg(info); // broadcast

#if (TEST_WS==1)
    // websocket server : receive/handle cmd from qiperfc
    m_wsserver = new WSServer(QIPERFD_WSPORT); // websocket listen
    connect(m_wsserver, &WSServer::actMessage, this ,&QIperfd::onWSactMessage);
    connect(m_wsserver, &WSServer::newClient, this ,&QIperfd::onNewClient);
    connect(this, &QIperfd::iperfStarted, m_wsserver, &WSServer::sendTextResult);
#endif
    // pipserver : interact with systemtray GUI (qiperftray)
    m_pserver=pserver;

    // iperf control interface, accept add/del iperf setting from remote
    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#if defined(Q_OS_ANDROID) || defined(Q_OS_WIN32)
    QString arch = QSysInfo::buildCpuArchitecture();
#endif
#if defined(Q_OS_LINUX)
    // linux/android path
#if !defined(Q_OS_ANDROID)
    QString tmp_path = QString(QDir::separator()) + "qiperf";
    QDir dir(tmp + tmp_path);
    if (!dir.exists())
    {
        dir.mkdir(tmp + tmp_path);
    }
#else
    QString tmp_path = "";
#endif
    //TODO: check we have newer version of iperf, remove old !!
    m_iperfexe2 = tmp + tmp_path + QDir::separator() + "iperf2";
    if (QFileInfo::exists(m_iperfexe2))
    {
        QFile::remove(m_iperfexe2);
    }
    m_iperfexe21 = tmp + tmp_path + QDir::separator() + "iperf2.1";
    if (QFileInfo::exists(m_iperfexe21))
    {
        QFile::remove(m_iperfexe21);
    }
    m_iperfexe3 = tmp + tmp_path + QDir::separator() + "iperf3";
    if (QFileInfo::exists(m_iperfexe3))
    {
        QFile::remove(m_iperfexe3);
    }

    QFile i2File(apppath+QDir::separator()+"linux"+QDir::separator()+"iperf2");
    if (i2File.exists()) {
        // make file execuable
        if (i2File.copy(m_iperfexe2)){
            QFile iperf2File(m_iperfexe2);
            iperf2File.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                  QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup |
                                  QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther);
        }else{
            qDebug() << "copy file " << " to " << m_iperfexe2 << " fail";
        }
    }else{
        qDebug() << i2File.fileName() << " NOT EXIST!!";
    }
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

    if (0){
// iperf2
#if defined(Q_OS_ANDROID)
    QFile i2File(":/android/" + arch + "/iperf");
#else
    QFile i2File(":/linux/iperf2");
#endif
    qDebug() << "iperf2 exist: " << i2File.exists();
    //    onLog("iperf2: " + i2File.fileName());
    if (!i2File.open(QIODevice::ReadOnly))
    {
        onLog("could not open " + i2File.fileName());
    }
    else
    {
        if (!i2File.copy(m_iperfexe2))
        {
            onLog("copy iperf2 " + i2File.fileName() + " to " + m_iperfexe2 + " fail");
        }
        else
        {
            // make file execuable
            QFile iperf2File(m_iperfexe2);
            iperf2File.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                      QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup |
                                      QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther);
            // QFileDevice::WriteOwner|QFileDevice::WriteGroup|QFileDevice::WriteOther
        }
    }
// iperf3
#if defined(Q_OS_ANDROID)
    QFile i3File(":/android/" + arch + "/iperf3");
#else
    QFile i3File(":/linux/iperf3");
#endif
    qDebug() << "iperf3 exist: " << i3File.exists();
    if (!i3File.open(QIODevice::ReadOnly))
    {
        onLog("could not open " + i3File.fileName());
    }
    else
    {
        if (!i3File.copy(m_iperfexe3))
        {
            onLog("copy iperf3 " + i3File.fileName() + " to " + m_iperfexe3 + " fail");
        }
        else
        {
            // make file execuable
            QFile iperf3File(m_iperfexe3);
            iperf3File.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                      QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup |
                                      QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther);
        }
    }
    }
#elif defined(Q_OS_WIN32)
    // windows, iperf files

    m_iperfexe2 = apppath + QDir::separator() + "windows" +QDir::separator() + "x86"+QDir::separator() + "iperf2.exe";
    m_iperfexe21 = apppath + QDir::separator() + "windows"+ QDir::separator() + "x86"+QDir::separator() + "iperf2.1.exe";
    m_iperfexe3 = apppath + QDir::separator() + "windows" +QDir::separator() + arch + QDir::separator() + "iperf3.exe";
#else
    qDebug() << " Not Support platform!!" << Qt::endl;
#endif

    // system service manager
    //     qDebug() << "finish contrust" << Qt::endl;
}

QIperfd::~QIperfd()
{
    qDebug() << "~QIperfd" << Qt::endl;
//    QString info = m_myinfo->disableInfo();
//    m_udpsrv->setSendMsg(info);
    savecfg();
}

//void QIperfd::closeEvent(QCloseEvent *event)
//{
//    Q_UNUSED(event)
//    //TODO: close app check
//    qDebug() << "closeEvent" <<  Qt::endl;
//    savecfg();
//}
void QIperfd::onLog(QString text)
{
    qInfo() << qApp->applicationPid() <<"," << text;
}

void QIperfd::loadcfg(QString apppath)
{
    onLog(",loadcfg: (apppath:" + apppath + ")");
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
    onLog("mgr_ifname: " + mgr_ifname + ", mgr_port: " + QString::number(mgr_port));
    cfg->endGroup();
}

void QIperfd::savecfg()
{
    //    qDebug()<< "savecfg" << Qt::endl;

    cfg->beginGroup("manager");
    cfg->setValue("ifname", mgr_ifname);
    cfg->setValue("port", mgr_port);
    cfg->endGroup();
    cfg->sync();
    qInfo() <<qApp->applicationPid() << ",savecfg:" << cfg->status() << Qt::endl;

}

//QList<QString> QIperfd::listInterfaces()
QStringList QIperfd::listInterfaces()
{
    //QList<QString> nslist;
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
            qDebug() << "getIfNameByHumanReadableName:" << ifname << " from: " <<name << Qt::endl;
            break;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return ifname;
}

int QIperfd::add(QString refrow, int version, QString m_cmd, QString args, uint port,
                 QString bndaddr, QString target,
                 QString parallel, QString protocal, bool bidir, bool reverse,
                 int interval)
{ // add a IperfWorker to run iperf server/client
    // TODO: check host/port used?
    QThread *iperf_th = new QThread();
    int idx = m_threads.count();
    m_threads.insert(idx, iperf_th);
    //    m_threads.append(iperf_th);
    //    int idx = m_threads.count()-1;
    IperfWorker *iperfer = new IperfWorker(idx, version, m_cmd, args, port,
                                           bndaddr, target, bidir, reverse,
                                           interval);
    iperfer->setRefRow(refrow);
    iperfer->setExtra(parallel, protocal);
//    connect(iperfer, &IperfWorker::onStdout, this, &QIperfd::readStdOut);
    connect(iperfer, &IperfWorker::onStderr, this, &QIperfd::onErrored);
    connect(iperfer, &IperfWorker::log, this, &QIperfd::onIperfLog);
    connect(iperfer, &IperfWorker::started, this, &QIperfd::onStarted);
    connect(iperfer, &IperfWorker::finished, this, &QIperfd::onFinished);
    connect(iperfer, &IperfWorker::onThroughput, this, &QIperfd::onThroughput);
    iperfer->moveToThread(iperf_th);
    connect(iperf_th, &QThread::started, iperfer, &IperfWorker::work);
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
    }else if (ver==static_cast<int>(IPERF_VER::V2)){
        cmd = m_iperfexe2;
    }else{
        qDebug() << "Not support Iperf version:" << ver << Qt::endl;
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

    //conver json data format to iperf args
    QString args;
    if (ver == static_cast<int>(IPERF_VER::V3)){
        args = m_iperfwrapper->toIperf3args(jsondata);
//        args = toIperf3args(jsondata);
    }else if (ver==static_cast<int>(IPERF_VER::V2)){
        qDebug() << "TODO convert json format to Iperf2 args";
    }else {
        qDebug() << "Not support Iperf version:" << ver;
        return -1;
    }
    return add(refrow, ver, cmd, args, port, binaddr,
               target, parallel, protocal, bidir, reverse, interval);
}

void QIperfd::del(int idx)
{
    //delete specify m_iperfworkers & m_threads
    //
    if (m_threads.contains(idx))
    {
        m_threads.remove(idx);
    }
    if (m_iperfworkers.contains(idx))
    {
        m_iperfworkers.remove(idx);
    }

    m_runstatus[idx]=0;
}

int QIperfd::addIperfServer(QString refrow, int version, uint port, QString bindHost)
{
    qDebug() << "addIperfServer:" << bindHost << ":" << port << Qt::endl;

    QString cmd;
    // add a iperf server
    if (version==static_cast<int>(IPERF_VER::V3)){
        cmd = m_iperfexe3;
    }else if (version==static_cast<int>(IPERF_VER::V2)){
        cmd = m_iperfexe2;
    }else{
        qDebug() << "Not support Iperf version:" << version << Qt::endl;
        return -1;
    }
    QString args="-s";
    if (!(bindHost=="")) {
        args.append("--bind");
        args.append(bindHost);
    }
    return add(refrow, version, cmd, args, port, bindHost);
}

int QIperfd::addIperfClient(QString refrow, int version, uint port, QString Host, QString iperfargs)
{
    qDebug() << "addIperfClient:" << Host << ":" << port << Qt::endl;
    QString cmd;
    // add a iperf server
    if (version==static_cast<int>(IPERF_VER::V3)){
        cmd = m_iperfexe3;
    }else if (version==static_cast<int>(IPERF_VER::V2)){
        cmd = m_iperfexe2;
    }else{
        qDebug() << "Not support Iperf version:" << version << Qt::endl;
        return -1;
    }
    QString args = iperfargs;
    return add(refrow, version, cmd, args, port);
}

void QIperfd::start(int idx)
{
    QThread *th = m_threads.value(idx);
    m_iperfworkers.value(idx)->setIperfLogPath(tmpfilepath+"/"+s_starttime);
    th->start();
}
void QIperfd::startAll()
{
    QString tmp = tmpfilepath+"/"+s_starttime;
    QDir d(tmp);
    if (!d.exists()){
        d.mkpath(tmp);
    }
    // start all thread
//    for (int i = 0; i < m_threads.count(); ++i)
    for (auto it = m_threads.begin(); it != m_threads.end(); ++it)
    {
//        start(i);
        start(it.key());
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}
void QIperfd::stop(int idx)
{
    if (isRunning(idx)){
        m_iperfworkers.value(idx)->setStop();
        //    iperfwork->setStop();
    }
}

void QIperfd::stopAll()
{
    // stop all iperfworker
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

void QIperfd::setManagerInterface(QString ifname)
{
    mgr_ifname = ifname;
//    qDebug() << "setManagerInterface:" << mgr_ifname << Qt::endl;
    savecfg();
    emit setMgrIfname(ifname);
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
        //        qDebug() << "TODO: send current status back to GUI" << Qt::endl;
        QVariantMap status;
        QVariantMap workers;
        workers.insert("iperfworkers", QString::number(m_iperfworkers.count()));
        status.insert("CMD", CMD_STATUS);
        status.insert(CMD_STATUS, workers);
        // TODO: any iperf running
        status.insert(CMD_RUNNING, QString::number(m_iperfworkers.count()));
        QJsonDocument jsonDocument = QJsonDocument::fromVariant(status);
        QString backmsg = jsonDocument.toJson(QJsonDocument::Compact).toStdString().c_str();
//        qDebug() << "send status (" << idx << "): " << backmsg  << Qt::endl;
        m_pserver->send_MessageBack(idx, backmsg);
    }
    else if (QString::compare(msg, CMD_IFNAMES, Qt::CaseInsensitive) == 0)
    {
        // get all interfaces names
        QJsonObject netObjs = m_myinfo->collectNetInfo();
        QVariantMap status;
        QVariantMap ifname;
        ifname.insert("ifnames", netObjs.keys());
        status.insert("CMD", CMD_IFNAMES);
        status.insert(CMD_IFNAMES, ifname);
        status.insert("ifname", mgr_ifname); // current manager ifname
        QJsonDocument jsonDocument = QJsonDocument::fromVariant(status);
        QString backmsg = jsonDocument.toJson(QJsonDocument::Compact).toStdString().c_str();
//        qDebug() << "send ifnames: (" << idx << "): " << backmsg  << Qt::endl;
        m_pserver->send_MessageBack(idx, backmsg);
    }
    else
    {
        qDebug() << "handle json: " << msg << Qt::endl;
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
                    cmd = m_iperfexe2;
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
                onLog("TODO: handle new json message:(" + QString(idx) + ")" + (msg));
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

    QString msg = QString(CMD_IPERF_ERRORED)+":"+QString::number(refrow);
    msg = msg + ":1:"+ text+":"+ipport; // error no, error
    m_wsserver->sendTextResult(msg);
    //remove error process??
    if (isRunning(m_idx)){
        stop(m_idx);
    }
    del(m_idx);
}

void QIperfd::onIperfLog(int idx, QString text)
{
//    qDebug() << "TODO: onIperfLog(" << QString::number(idx) << "):" << text << Qt::endl;
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
    onLog("TODO: onFinished: " + msg);
    m_wsserver->sendTextResult(msg);
    if (!filename.isEmpty()){
        if(QFileInfo::exists(filename)){
//            m_fileclient->sendFile(filename);
            m_fileclient->enqueueFile(filename);
//            m_wsserver->addFileToSend(filename);
        }else{
            qDebug() << "file to send not Exist: " << filename;
        }
    }
    del(idx);

}

void QIperfd::onThroughput(int idx, QString sInterval, QString data)
{
//    if (bReportTPData){
        m_wsserver->sendTextResult(QString(CMD_IPERF_TP_DATA)+":"+
                               QString::number(idx)+":"+
                               sInterval+":"+
                               data);
//    }
}

void QIperfd::onQuit()
{
    onLog("onQuit");
    savecfg();
    qApp->quit();
}

void QIperfd::onWSactMessage(QString msg)
{
    //handle act message from websocket
    int cut = msg.indexOf(':', 0);
    QString act = msg.left(cut);
    qDebug()<< "onWSactMessage: " << act;
    msg = msg.right(msg.length()-cut-1);
    // expect in json format
    if (act.startsWith(CMD_IPERF_ADD)){
        QJsonParseError error;
        cut = msg.indexOf(':', 0);
        QString refrow = msg.left(cut);
        msg = msg.right(msg.length()-cut-1);

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
        bReportTPData = true;
        qInfo() << "SET to Report throughput data: " << msg;
    }else if (act.startsWith(CMD_IPERF_UNREG)){
        QStringList d = msg.split(":");
        QString starttime = d[0];
        QString bindkey = d[1];
        bReportTPData = false;
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
    }else {
        qDebug() << " Unknown action:" << act  << " \n==========\n" << msg;
        qDebug() << "\n==========";
    }
}

void QIperfd::onNewClient(QHostAddress addr)
{
//    qDebug() << "onNewClient: "  << addr.toString();
    if (m_fileclient){
        qDebug() << "m_fileclient exist: " << m_fileclient->getTargetAddress() << " new: " << addr.toString();
    }
    //TODO: multi file client
    m_fileclient = new FileClient(QIPERF_FILEPORT, addr.toString());
}
