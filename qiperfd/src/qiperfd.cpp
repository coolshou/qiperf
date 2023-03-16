#include "qiperfd.h"
#include "../src/comm.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QNetworkInterface>

#include <QDebug>

QIperfd::QIperfd(QObject *parent)
    : QObject{parent}
{
    //TODO: setting
    QSettings cfg = QSettings(QSettings::NativeFormat, QSettings::UserScope,
        QIPERF_ORG, QIPERFD_NAME);
    QString apppath = qApp->applicationDirPath();
    loadcfg(apppath);
//    qDebug() << "start UdpSrv" << Qt::endl;
    //
    m_udpsrv = new UdpSrv(QIPERFD_BPORT, mgr_ifname);
    m_udpsrv->collectInfo();

//    qDebug() << "start PipeServer" << Qt::endl;
    //tray GUI interaction interface
    m_pserver=new PipeServer(QIPERFD_NAME, NULL);
    connect(m_pserver, SIGNAL(newMessage(int,QString)), this, SLOT(onNewMessage(int,QString)));
    if (m_pserver->init()){
        qDebug() << "PipeServer start fail" << Qt::endl;
    }
    //iperf control interface, accept add/del iperf setting from remote

    qDebug() << "init path" << Qt::endl;

    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString arch = QSysInfo::buildCpuArchitecture();
#if defined (Q_OS_LINUX)
    // linux/android path
    m_iperfexe2 = tmp +"/iperf2";
    if (QFileInfo::exists(m_iperfexe2)){
        QFile::remove(m_iperfexe2);
    }
    m_iperfexe21 = tmp +"/iperf21";
    if (QFileInfo::exists(m_iperfexe21)){
        QFile::remove(m_iperfexe21);
    }
    m_iperfexe3 = tmp +"/iperf3";
    if (QFileInfo::exists(m_iperfexe3)){
        QFile::remove(m_iperfexe3);
    }
//iperf2
#if defined (Q_OS_ANDROID)
    QFile i2File(":/android/"+arch+"/iperf");
#else
    QFile i2File(":/linux/"+arch+"/iperf2");
#endif
    //    onLog("iperf2: " + i2File.fileName());
    if (!i2File.open(QIODevice::ReadOnly)){
        onLog("could not open " + i2File.fileName()) ;
    }else{
        if (!i2File.copy(m_iperfexe2)){
            onLog("copy iperf3 "+ i2File.fileName()+ " to "+ m_iperfexe2 + " fail");
        } else {
            // make file execuable
            QFile iperf2File(m_iperfexe2);
            iperf2File.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner| QFileDevice::ExeOwner|
                                      QFileDevice::ReadGroup | QFileDevice::WriteGroup| QFileDevice::ExeGroup|
                                      QFileDevice::ReadOther | QFileDevice::WriteOther| QFileDevice::ExeOther);
            // QFileDevice::WriteOwner|QFileDevice::WriteGroup|QFileDevice::WriteOther
        }
    }
//iperf3
#if defined (Q_OS_ANDROID)
    QFile i3File(":/android/"+arch+"/iperf3");
#else
    QFile i3File(":/linux/"+arch+"/iperf3");
#endif
    if (!i3File.open(QIODevice::ReadOnly)){
        onLog("could not open " + i3File.fileName()) ;
    }else{
        if (!i3File.copy(m_iperfexe3)){
            onLog("copy iperf3 "+ i3File.fileName()+ " to "+ m_iperfexe3 + " fail");
        } else {
            // make file execuable
            QFile iperf3File(m_iperfexe3);
            iperf3File.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner| QFileDevice::ExeOwner|
                                      QFileDevice::ReadGroup | QFileDevice::WriteGroup| QFileDevice::ExeGroup|
                                      QFileDevice::ReadOther | QFileDevice::WriteOther| QFileDevice::ExeOther);
        }
    }
#endif
#if defined (Q_OS_WIN32)
    //windows, multi files

    m_iperfexe2 = apppath + "/windows/x86/iperf2.exe";
    m_iperfexe21 = apppath + "/windows/x86/iperf21.exe";
    m_iperfexe3 = apppath + "/windows/" +arch+ "/iperf3.exe";
#endif

    //system service manager
//    qDebug() << "finish contrust" << Qt::endl;
}

void QIperfd::onLog(QString text)
{
    qDebug() << "log:" << text << Qt::endl;
}

void QIperfd::loadcfg(QString apppath)
{
    cfg.beginGroup("main");
    cfg.setValue("Path", apppath);
    cfg.endGroup();
    cfg.sync();

    listInterfaces();
    //load config setting
    cfg.beginGroup("manager");
    mgr_ifname = cfg.value("ifname", "eth0").toString();
    mgr_port = cfg.value("port", QIPERFD_PORT).toInt();
    cfg.endGroup();
}

void QIperfd::savecfg()
{
    cfg.beginGroup("manager");
    cfg.setValue("ifname", mgr_ifname);
    cfg.setValue("port", mgr_port);
    cfg.endGroup();
    cfg.sync();
}

QList<QString> QIperfd::listInterfaces()
{
    QList<QString> nslist;

    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface interface,list) //遍歷每一個網路介面
    {
        if ((interface.type() == QNetworkInterface::Ethernet) ||
                (interface.type() == QNetworkInterface::Wifi)) {
            nslist << interface.name();
        }
    }
    return nslist;
}

QString QIperfd::getInterfaceAddr(QString ifname)
{
    QString tmp="";
    //get first addr of an interface
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface interface,list) //遍歷每一個網路介面
    {
        if (ifname.compare(interface.name())==0) {
            if ((interface.type() == QNetworkInterface::Ethernet) ||
                    (interface.type() == QNetworkInterface::Wifi)) {
    //            nslist << interface.name();
                QList<QNetworkAddressEntry> entryList= interface.addressEntries();
                //only return first address
                if (entryList.length()>0){
                    QNetworkAddressEntry entry= entryList.at(0);
                    tmp = entry.ip().toString();
                }
                break;
            }
        }
    }
    return tmp;
}

void QIperfd::setMgr_ifname(QString ifname)
{
    mgr_ifname = ifname;
}

void QIperfd::add(int version, QString m_cmd, QString args, int port)
{
    //TODO: check port used?
    QThread *iperf_th = new QThread();
    int idx = m_threads.count();
    m_threads.insert(idx, iperf_th);
//    m_threads.append(iperf_th);
//    int idx = m_threads.count()-1;
    IperfWorker *iperfer = new IperfWorker(idx, version, m_cmd, args, port);
    connect(iperfer, SIGNAL(onStdout(int,QString)), this, SLOT(readStdOut(int,QString)));
    connect(iperfer, SIGNAL(onStderr(int,QString)), this, SLOT(readStdErr(int,QString)));
    connect(iperfer, SIGNAL(log(int,QString)), this, SLOT(onIperfLog(int,QString)));
    connect(iperfer, SIGNAL(started(int)), this, SLOT(onStarted(int)));
    connect(iperfer, SIGNAL(finished(int,int,int)), this, SLOT(onFinished(int,int,int)));
    iperfer->moveToThread(iperf_th);
    connect(iperf_th, SIGNAL(started()), iperfer, SLOT(work()));

//    m_iperfworkers.append(iperfer);
    m_iperfworkers.insert(idx, iperfer);
}

void QIperfd::start()
{
    //start all thread
    for( int i=0; i< m_threads.count(); ++i )
    {
        QThread *th = m_threads.value(i);
        QString id= QString( "%1" ).arg((long)(th->currentThreadId()), 16);
        qDebug() << "run :" << id << Qt::endl;
        th->start();
    }
}

void QIperfd::stop()
{
    //stop all iperfworker
    for( int i=0; i< m_iperfworkers.count(); ++i )
    {
        IperfWorker *iperfwork = m_iperfworkers.value(i);
        iperfwork->setStop();
    }
}

void QIperfd::onNewMessage(int idx, const QString msg)
{
    qDebug() << "TODO: handle new message:("<<idx<<")" <<  (msg) << Qt::endl;
    if (QString::compare(msg, CMD_OK, Qt::CaseInsensitive)==0){
        return;
    }
    if (QString::compare(msg, CMD_STATUS, Qt::CaseInsensitive)==0){
        //get current all iperf status
        qDebug() << "TODO: send current status back to GUI" << Qt::endl;
        m_pserver->send_MessageBack(idx, "current iperfworkers:" + QString::number(m_iperfworkers.count()));
    } else {
        //json format message
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError){
            QVariantMap result = doc.toVariant().toMap();
            QString act = result["Action"].toString();
            if (QString::compare(act, CMD_IPERF_ADD, Qt::CaseInsensitive)==0){


                QVariantMap iperf_args = result["iperf"].toMap();
                int ver = iperf_args["version"].toInt();
                QString cmd;
                if (ver==(int)IPERF_VER::V3){
                    cmd = m_iperfexe3;
                }else{
                    cmd = m_iperfexe2;
                }
                int port = iperf_args["port"].toInt();
                QString args;
                foreach (QVariant arg, iperf_args["cmd_args"].toList()){
                    qDebug() << "arg: " <<arg.toString() << Qt::endl;
                    args = " " + arg.toString();
                }
                qDebug() << "add iperf: "<< args << Qt::endl;
                add(ver, cmd, args, port);
            }else if (QString::compare(act, CMD_IPERF_START, Qt::CaseInsensitive)==0){
                qDebug() << "Start all iperfs" << Qt::endl;
                start();
            }else if (QString::compare(act, CMD_IPERF_STOP, Qt::CaseInsensitive)==0){
                qDebug() << "Stop all iperfs" << Qt::endl;
                stop();
            }

        } else {
//            qFatal(error.errorString().toUtf8().constData());
            qDebug() << "onNewMessage: ERROR: " << error.errorString() << "\nparser json: "<< msg.toUtf8() << Qt::endl;
            exit(1);
        }
    }
}

void QIperfd::readStdOut(int idx, QString text)
{
    qDebug() << "TODO: readStdOut("<<idx<<"):" <<text<< Qt::endl;
}

void QIperfd::readStdErr(int idx, QString text)
{
    qDebug() << "TODO: readStdErr("<<idx<<"):" <<text<< Qt::endl;
}

void QIperfd::onIperfLog(int idx, QString text)
{
    qDebug() << "TODO: onIperfLog("<<idx<<"):" <<text<< Qt::endl;
    onLog("("+QString(idx)+")"+text+"");
}

void QIperfd::onStarted(int idx)
{
    qDebug() << "TODO: onStarted("<<idx<<"):" << Qt::endl;
}

void QIperfd::onFinished(int idx, int exitCode, int exitStatus)
{
    qDebug() << "TODO: onFinished("<<idx<<"):" <<exitCode << ":" << exitStatus << Qt::endl;
    if (m_threads.contains(idx)){
        m_threads.remove(idx);
    }
    if (m_iperfworkers.contains(idx)){
        m_iperfworkers.remove(idx);
    }
//    m_threads.removeAt(idx);
//    m_iperfworkers.removeAt(idx);
}
