#include <QCoreApplication>
#include <QStandardPaths>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QNetworkInterface>
#include <QDir>

#include "qiperfd.h"
#include "../src/comm.h"

#include <QDebug>

QIperfd::QIperfd(PipeServer *pserver, QObject *parent)
    : QObject{parent}
{
    // TODO: setting
    cfg = new QSettings(QSettings::IniFormat, QSettings::SystemScope,
                              QIPERF_ORG, QIPERFD_NAME);
    qInfo() << qApp->applicationPid() <<",cfg filename:" << cfg->fileName();
    //SystemScope: /etc/xdg/xdg-lxqt/alphanetworks/qiperfd.conf
        //sudo =>      /etc/xdg/alphanetworks/qiperfd.conf
    //UserScope: /home/jimmy/.config/alphanetworks/qiperfd.conf
        //sudo =>       /root/.config/alphanetworks/qiperfd.conf

    QFileInfo fi(cfg->fileName());
    if (!QDir(fi.absolutePath()).exists()){
        QDir().mkdir(fi.absolutePath());
    }
    QString apppath = qApp->applicationDirPath();
    loadcfg(apppath);
    //    qDebug() << "start UdpSrv" << Qt::endl;
    //
    m_myinfo = new MyInfo(mgr_ifname);
    connect(this, &QIperfd::setMgrIfname, m_myinfo, &MyInfo::setIfname);
    QString info = m_myinfo->collectInfo();

    m_udpsrv = new UdpSrv(QIPERFD_BPORT, mgr_ifname, m_myinfo);
    connect(this, &QIperfd::setMgrIfname, m_udpsrv, &UdpSrv::setIfname);
    m_udpsrv->setSendMsg(info); // broadcast

#if (TEST_WS==1)
    //
    m_wsserver = new WSServer(QIPERFD_WSPORT); // websocket listen
    connect(m_wsserver, &WSServer::actMessage, this ,&QIperfd::onWSactMessage);

#endif
    m_pserver=pserver;
//    //TODO: why following did not work??
    if (!connect(m_pserver, &PipeServer::pipeMessage, this, &QIperfd::onPipeMessage)){
        qInfo() << "connect pipeMessage fail";
    }

    // systemtray GUI interaction interface
    // iperf control interface, accept add/del iperf setting from remote

    qInfo() << qApp->applicationPid() <<",init path & files";

    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#if defined(Q_OS_ANDROID) || defined(Q_OS_WIN32)
    QString arch = QSysInfo::buildCpuArchitecture();
#endif
#if defined(Q_OS_LINUX)
    // linux/android path
#if !defined(Q_OS_ANDROID)
    QString tmp_path = "/qiperf";
    QDir dir(tmp + tmp_path);
    if (!dir.exists())
    {
        dir.mkdir(tmp + tmp_path);
    }
#else
    QString tmp_path = "";
#endif
    m_iperfexe2 = tmp + tmp_path + "/iperf2";
    if (QFileInfo::exists(m_iperfexe2))
    {
        QFile::remove(m_iperfexe2);
    }
    m_iperfexe21 = tmp + tmp_path + "/iperf2.1";
    if (QFileInfo::exists(m_iperfexe21))
    {
        QFile::remove(m_iperfexe21);
    }
    m_iperfexe3 = tmp + tmp_path + "/iperf3";
    if (QFileInfo::exists(m_iperfexe3))
    {
        QFile::remove(m_iperfexe3);
    }
// iperf2
#if defined(Q_OS_ANDROID)
    QFile i2File(":/android/" + arch + "/iperf");
#else
    QFile i2File(":/linux/iperf2");
#endif
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
#elif defined(Q_OS_WIN32)
    // windows, multi files

    m_iperfexe2 = apppath + "/windows/x86/iperf2.exe";
    m_iperfexe21 = apppath + "/windows/x86/iperf2.1.exe";
    m_iperfexe3 = apppath + "/windows/" + arch + "/iperf3.exe";
#else
    qDebug() << " Not Support platform!!" << Qt::endl;
#endif

    // system service manager
    //     qDebug() << "finish contrust" << Qt::endl;
}

QIperfd::~QIperfd()
{
//    QString info = m_myinfo->disableInfo();
//    m_udpsrv->setSendMsg(info);
    savecfg();
}

void QIperfd::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    //TODO: close app check
    qInfo() << "closeEvent";
    savecfg();
}
void QIperfd::onLog(QString text)
{
    qInfo() << qApp->applicationPid() <<", " << text;
}

void QIperfd::loadcfg(QString apppath)
{
    onLog(",loadcfg: (apppath:" + apppath + ")");
    cfg->beginGroup("main");
    cfg->setValue("Path", apppath);
    cfg->endGroup();
    cfg->sync();

    listInterfaces();
    // load config setting
    cfg->beginGroup("manager");
    mgr_ifname = cfg->value("ifname", "eth0").toString();
    mgr_port = cfg->value("port", QIPERFD_PORT).toInt();
    qInfo() << "mgr_ifname: " << mgr_ifname << ", mgr_port: " <<mgr_port ;
    cfg->endGroup();
//    setManagerInterface(mgr_ifname);
}

void QIperfd::savecfg()
{
    //    qDebug()<< "savecfg" << Qt::endl;

    cfg->beginGroup("manager");
    cfg->setValue("ifname", mgr_ifname);
    cfg->setValue("port", mgr_port);
    cfg->endGroup();
    cfg->sync();
    qInfo() <<qApp->applicationPid() << ",savecfg:" << cfg->status();
    //    qDebug()<< "savecfg end" << Qt::endl;
}

QList<QString> QIperfd::listInterfaces()
{
    QList<QString> nslist;

    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface interface, list) // 遍歷每一個網路介面
    {
        if ((interface.type() == QNetworkInterface::Ethernet) ||
            (interface.type() == QNetworkInterface::Wifi))
        {
            nslist << interface.name();
        }
    }
    return nslist;
}

QString QIperfd::getInterfaceAddr(QString ifname)
{
    QString tmp = "";
    // get first addr of an interface
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface interface, list) // 遍歷每一個網路介面
    {
        if (ifname.compare(interface.name()) == 0)
        {
            if ((interface.type() == QNetworkInterface::Ethernet) ||
                (interface.type() == QNetworkInterface::Wifi))
            {
                //            nslist << interface.name();
                QList<QNetworkAddressEntry> entryList = interface.addressEntries();
                // only return first address
                if (entryList.length() > 0)
                {
                    QNetworkAddressEntry entry = entryList.at(0);
                    tmp = entry.ip().toString();
                }
                break;
            }
        }
    }
    return tmp;
}

QString QIperfd::getManagerInterface()
{
    return mgr_ifname;
}

int QIperfd::add(int version, QString m_cmd, QString args, uint port)
{ // add a IperfWorker to run iperf server/client
    // TODO: check host/port used?
    QThread *iperf_th = new QThread();
    int idx = m_threads.count();
    m_threads.insert(idx, iperf_th);
    //    m_threads.append(iperf_th);
    //    int idx = m_threads.count()-1;
    IperfWorker *iperfer = new IperfWorker(idx, version, m_cmd, args, port);
    connect(iperfer, &IperfWorker::onStdout, this, &QIperfd::readStdOut);
    connect(iperfer, &IperfWorker::onStderr, this, &QIperfd::readStdErr);
    connect(iperfer, &IperfWorker::log, this, &QIperfd::onIperfLog);
    connect(iperfer, &IperfWorker::started, this, &QIperfd::onStarted);
    connect(iperfer, &IperfWorker::finished, this, &QIperfd::onFinished);
    iperfer->moveToThread(iperf_th);
    connect(iperf_th, &QThread::started, iperfer, &IperfWorker::work);

    //    m_iperfworkers.append(iperfer);
    m_iperfworkers.insert(idx, iperfer);
    return idx;
}

int QIperfd::addIperfServer(int version, uint port, QString bindHost)
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
    return add(version, cmd, args, port);
}

int QIperfd::addIperfClient(int version, uint port, QString Host, QString iperfargs)
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
    return add(version, cmd, args, port);
}

void QIperfd::start(int idx)
{
    QThread *th = m_threads.value(idx);
    // QString id= QString( "%1" ).arg(reinterpret_cast<long>(th->currentThreadId()), 16);
    QString id = QString("%1").arg(quintptr(th->currentThreadId()), 16, 16, QLatin1Char('0'));
    qDebug() << "run thread id:" << id << Qt::endl;
    th->start();

}
void QIperfd::startAll()
{
    m_starttime = QDateTime().currentDateTime();
    // start all thread
    for (int i = 0; i < m_threads.count(); ++i)
    {
        start(i);
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
    for (int i = 0; i < m_iperfworkers.count(); ++i)
    {
        stop(i);
    }
}

bool QIperfd::isRunning(int idx)
{
    if (m_iperfworkers.contains(idx)) {
        return m_iperfworkers.value(idx)->isRunning();
    }
    return false;
}

void QIperfd::setManagerInterface(QString interface)
{
    mgr_ifname = interface;
//    savecfg();
    emit setMgrIfname(interface);
}

void QIperfd::onPipeMessage(int idx, const QString msg)
{
//    qInfo() << "(" << idx <<")onNewMessage: = " << msg;
    onLog("(" + QString(idx) + ")onNewMessage: = " + msg);
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
        m_pserver->send_MessageBack(idx, jsonDocument.toJson(QJsonDocument::Compact).toStdString().c_str());
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
        m_pserver->send_MessageBack(idx, jsonDocument.toJson(QJsonDocument::Compact).toStdString().c_str());
    }
    else
    {
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
                QVariantMap iperf_args = result["iperf"].toMap();
                int ver = iperf_args["version"].toInt();
                QString cmd;
                if (ver == static_cast<int>(IPERF_VER::V3))
                {
                    cmd = m_iperfexe3;
                }
                else
                {
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
                add(ver, cmd, args, port);
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
    onLog("TODO: readStdOut(" + QString(idx) + "):" + text);
}

void QIperfd::readStdErr(int idx, QString text)
{
    onLog("TODO: readStdErr(" + QString(idx)+ "):" + text);
}

void QIperfd::onIperfLog(int idx, QString text)
{
    qDebug() << "TODO: onIperfLog(" << idx << "):" << text << Qt::endl;
    onLog("(" + QString(idx) + ")" + text + "");
}

void QIperfd::onStarted(int idx)
{
    onLog("TODO: onStarted(" + QString(idx) + "):");
    m_runstatus[idx]=1;
}

void QIperfd::onFinished(int idx, int exitCode, int exitStatus)
{
    onLog("TODO: onFinished(" + QString(idx) + "):" +QString(exitCode)+  ":" + QString(exitStatus));
    if (m_threads.contains(idx))
    {
        m_threads.remove(idx);
    }
    if (m_iperfworkers.contains(idx))
    {
        m_iperfworkers.remove(idx);
    }
    //    m_threads.removeAt(idx);
    //    m_iperfworkers.removeAt(idx);
    m_runstatus[idx]=0;
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
    // expect in json format
    onLog("TODO: onWSactMessage:" + msg);

}
