#include "qiperfc.h"
#include "ui_qiperfc.h"
#include "../src/comm.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QStringLiteral>
#include <QUrl>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSaveFile>
#include <QThread>
#include <QCoreApplication>
#include <QEventLoop>
#include <QTreeView>
#include <QToolTip>
#include <QAction>

#include "endpointact.h"
#include "tp.h"
#include "versions.h"

#include <QDebug>



QIperfC::QIperfC(QString logpath, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    QString settingfilepath =  QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    // /home/jimmy/.local/share/alphanetworks/qiperfconsole
    QDir d{settingfilepath};
    if (!d.exists()){
        if(!d.mkpath(settingfilepath)){
            qDebug() << "ERROR: mkdir " + settingfilepath + " Fail";
        }
    }
    QString settingfilename = settingfilepath + QDir::separator() + QIPERFC_NAME + ".ini";
    m_TestStartTime = QDateTime();
    m_settings=new QSettings(settingfilename, QSettings::IniFormat);
    m_clipboard = QApplication::clipboard();
    ui->setupUi(this);

    m_logpath = logpath + "data";
    QDir logdir(m_logpath);
    if (!logdir.exists()){
        qDebug() << "create path: " << m_logpath;
        logdir.mkpath(".");
    }
    m_dlgtest = new DlgTest();
    m_frm_option = new dlgOption(m_settings);
    initStatusbar();
    loadSettings();
    //UI actions
    initActions();
    initMenus();
    initToolbar();
    initThroughputChart();
    initPingChart();
    connect(this, &QIperfC::errorStop, this, &QIperfC::onErrorStop);

    iTimeout = 10*1000;//10sec
    //
    m_qipconfig = new QIPConfig(logdir.absolutePath());
    connect(m_qipconfig, &QIPConfig::updateDataPath, this, &QIperfC::onUpdateDataPath);
    connect(m_qipconfig, &QIPConfig::updateTPCfg, this, &QIperfC::onUpdateTPCfg);
    connect(m_qipconfig, &QIPConfig::updateStartDateTime, this, &QIperfC::setStartTime);
    // connect(m_qipconfig, &QIPConfig::updateTPDatas, m_tpmgr, &TPMgr::onUpdateTPDatas);
    connect(m_qipconfig, &QIPConfig::updateTPAvg, m_tpmgr, &TPMgr::addTPdata); // this only set last avg, which may cause min/max value wrong!!
    connect(m_qipconfig, &QIPConfig::updateTPDatas, m_tpplot, &TPPlot::onUpdateTPDatas);
    connect(m_qipconfig, &QIPConfig::progress, this, &QIperfC::onProgress);
    connect(m_qipconfig, &QIPConfig::updateStartDateTime, m_tpplot, &TPPlot::setStartTime);
    QString proxyhost="";
    quint16 proxyport=0;
    if (m_qipconfig->detectSystemProxy(proxyhost, proxyport)){
        onError("Detect system have proxy setting (proxy="+ proxyhost +":"+QString::number(proxyport)+"), which may cause qiperfd control problem!!");
    }
    m_endpointmgr = new EndPointMgr(this);
    m_frm_qiperfds = new FormQIperfds();
    m_frm_qiperfds->setModel(m_endpointmgr);

    dlgiperf = new DlgIperf(m_tpmgr, this);

    //
    m_receiver = new UdpReceiver(QIPERFD_BPORT,this);
    connect(m_receiver, &UdpReceiver::notice, this, &QIperfC::on_notice);
    connect(m_receiver, &UdpReceiver::error, this, &QIperfC::onError);

    // control local qiperfd?
//    pclient = new PipeClient(QIPERFD_NAME);
//    connect(pclient, SIGNAL(newMessage(QString)), this, SLOT(onNewMessage(QString)));
//    pclient->SetAppHandle(qApp);

    m_dlgrecord = new DlgRecord(this);
    m_fileserver = new FileServer(QIPERF_FILEPORT);
#if (TEST_ICMP==1)
    dp = new DlgPing(this);
#endif
    m_dlgshowlog=new DlgShowLog(logpath+QIPERFC_NAME+".log");
    connect(this, &QIperfC::closeAll, m_dlgshowlog, &DlgShowLog::close);
//    connect(this, &QIperfC::closeAll, dp, &DlgPing::close); // model mode, no need
    connect(this, &QIperfC::closeAll, m_dlgrecord, &DlgRecord::close);
//    connect(this, &QIperfC::closeAll, dlgiperf, &DlgIperf::close);// model mode, no need
    connect(this, &QIperfC::closeAll, m_frm_qiperfds, &FormQIperfds::close);
//    connect(this, &QIperfC::closeAll, m_frm_option, &dlgOption::close);// model mode, no need
    connect(this, &QIperfC::closeAll, m_dlgtest, &DlgTest::close);
}

QIperfC::~QIperfC()
{
//    qDebug() << "~QIperfC";
    delete ui;
}

bool QIperfC::load(QString filename)
{    //load test config file

    if (m_tpmgr->rootChildCount()>0) {
        QMessageBox msgBox;
        msgBox.setText("Clear data before load config");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        switch (ret) {
          case QMessageBox::Save:
              // Save was clicked
              onSave();
              break;
          case QMessageBox::Discard:
              // Don't Save was clicked
              on_Clear();
              break;
          case QMessageBox::Cancel:
              // Cancel was clicked
              return false;
              //break;
          default:
              // should never be reached
              break;
        }
    }
    if (m_qipconfig->loadFromFile(filename)){
        return true;
    }else{
        qDebug() << "load file " << filename << " Fail!!";
    }
    return false;
}

bool QIperfC::save(QString filename)
{
    //prepare throughput config data
    if (m_tpmgr->rootChildCount()>0) {
        QByteArray b = m_tpmgr->savedata();
        QStringList pcs = m_tpmgr->getPCs();
        QString env= m_endpointmgr->getPCsInfo(pcs);
//        qDebug() << "env: " << env;
        QString starttime="";
        if (m_TestStartTime.isValid()){
            starttime = m_TestStartTime.toString(DATETIME_NOW_FORMAT);
            QString tmp = m_logpath + QDir::separator() + starttime;
            QDir d(tmp);
            QStringList filelist;
            foreach(auto s, d.entryList(QDir::Files)){
                filelist.append(tmp+ QDir::separator()+s);
            }
            m_qipconfig->setTPCfg(b, env, starttime, filelist);
        }else{
            m_qipconfig->setTPCfg(b, env);
        }
        m_qipconfig->saveToFile(filename);
        return true;
    }else {
        qDebug() << "NO throughput config to save";
        return false;
    }
}

QString QIperfC::getNowString()
{
    return QDateTime::currentDateTime().toString(DATETIME_NOW_FORMAT);
}

void QIperfC::onNewMessage(const QString msg)
{
    m_dlgtest->append(msg);
}

void QIperfC::onNew()
{
    ui->actionSave->setEnabled(false);
    on_Clear();
    if (m_tpmgr->rootChildCount()>0) {
        //this will clear all item include root!!
        m_tpmgr->reset();
    }/*else{
        qDebug() << "onNew rootChildCount No child";
    }*/
}

void QIperfC::onOpen()
{
//    onNew();
    QString path;
    if (!m_oldsavepath.isNull()){
        path = m_oldsavepath;
    }else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open QIperf file"), path , tr(QIPERF_EXT_FILTER));
    QFileInfo fi(fileName);
    QString ext = fi.suffix();
    if (ext.compare(QIPERF_EXT)!=0){
        qDebug() << "Not support file format: " << fileName;
        return;
    }
    doClear();
    onNew();
    if (load(fileName)){
        m_oldsavepath = fi.path();
    }
    ui->actionSave->setEnabled(true);
}

void QIperfC::onSave()
{
    QString path;
    if (!m_oldsavepath.isNull()){
        path = m_oldsavepath;
    }else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString fileName = QFileDialog::getSaveFileName(this,
             tr("Save QIperf "), path, tr(QIPERF_EXT_FILTER));
    QFileInfo fi(fileName);
    QString ext = fi.suffix();
    if (ext.compare(QIPERF_EXT)!=0){
        fileName = fi.path() + fi.baseName() + "."+ QIPERF_EXT;
    }
//    qInfo() << "save file: " << fileName ;
    if (save(fileName)){
        m_oldsavepath = fi.path();
    }

}

void QIperfC::onImportIperf3Log()
{
    qDebug() << "TODO:  Import Iperf3 Log file to throughput chart";
    QString path;
    if (!m_oldsavepath.isNull()){
        path = m_oldsavepath;
    }else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open Iperf3 log file"), path , tr(ALL_EXT_FILTER));
    if(!m_qipconfig->importIperf3Log(fileName)){
        qDebug() << "Import file: " << fileName << " Fail!!";
    }
}

bool QIperfC::on_Clear()
{
    // this will clean iperf test pair config
    return onClear();
}

void QIperfC::onAddIperf()
{
    // on_pair_add
    dlgiperf->updateUI();
    dlgiperf->setExcIdx(QModelIndex());//new
    int rc = dlgiperf->exec();// show dlgiperf
    if (rc == QDialog::Accepted){
        QString rs= dlgiperf->getJsonCfg();
//        qDebug()<< "on_pairAdd: \n" << rs;
        m_tpmgr->add(rs);
        ui->actionSave->setEnabled(true);
    }

}
void QIperfC::onAddPing()
{
    QString strJson;
#if (TEST_ICMP==1)
    if (dp->exec()== QDialog::Accepted){
        strJson = dp->getJsonstr();
        qDebug() << "strJson:" << strJson;
        //TODO: add to ping treeview/chart
        ui->actionSave->setEnabled(true);
    }
#endif
}

void QIperfC::onError(QString msg)
{
    QMessageBox::warning(this, "ERROR", msg);
}

void QIperfC::onPairEdit()
{
    // TODO: edit
    QModelIndex idx = ui->tv_throughput->selectionModel()->currentIndex();
//    qDebug() << "on_pairEdit: " << cur;
    onItemDClicked(idx);
}

void QIperfC::onPairDelete()
{
    QModelIndex cur = ui->tv_throughput->selectionModel()->currentIndex();
    TP *tp = m_tpmgr->getItem(cur);
    foreach(TP *p, tp->getChilds()){
        m_tpplot->del(p->getID());
    }
    m_tpmgr->removeRow(cur.row());
}

void QIperfC::onPairSwap()
{
    QModelIndexList mls= ui->tv_throughput->selectionModel()->selectedRows();
    foreach (QModelIndex midx, mls) {
        m_tpmgr->swapDirection(midx);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

void QIperfC::onPairSwapIP()
{
    QModelIndexList mls= ui->tv_throughput->selectionModel()->selectedRows();
    foreach (QModelIndex midx, mls) {
        m_tpmgr->swapIPDirection(midx);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

void QIperfC::onStart()
{
    if (!onClear()){
        return;
    }
    bUserStop = false;
    ui->actionShowLog->setEnabled(true);
    ui->actionSave->setEnabled(true);
    resetError();
    //TODO: clear old test record!!
    m_status_server.clear();
    m_status_client.clear();

    m_TestStartTime = QDateTime::currentDateTime();
    m_tpplot->setStartTime(m_TestStartTime);
    QString startTime = m_TestStartTime.toString(DATETIME_NOW_FORMAT);
    m_datapath = m_logpath + QDir::separator() + startTime;
    QDir d(m_datapath);
    if (!d.exists()){
        d.mkpath(".");
    }
    // qDebug() << "QIperfC::onStart(): m_datapath" << m_datapath;
    m_fileserver->setRootPath(m_datapath);

    emit updateStarttime(startTime);
    if (m_tpmgr->rootChildCount()>0) {
        updateRunStatus(true);
        //start test
        // list of throughput test pair
        QList<TP *> tps = m_tpmgr->getChilds();
        QString s; // websocket url
        QString cmd;
        qint64 rs=0;
        int maxtestduration=0; // max wait test time
        int iwait=0;
        int idelaytime=0;
        int itimeout;
        int refrow;
        foreach (TP *tp, tps) {
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            if (tp->getEnabled()){
                //RPC to control all server endpoint (iperf server)
                iwait = tp->getWaitTime();
                if (iwait> maxtestduration){
                    maxtestduration = iwait+5;
                }
                idelaytime = tp->getDelaytime();
                if (idelaytime>0) {
                    maxtestduration = maxtestduration + idelaytime;
                }
                refrow = tp->row();
                QString serverIP = tp->getMgrServer();
                //TODO: detect manager server is pingable
                if (!m_wss.contains(serverIP)) {
                    //TODO: can not work with interface with DHCP under Windows??
                    s = "ws://"+serverIP+":"+QString::number(QIPERFD_WSPORT);
                    // qDebug() << "server websocket url: " << s;
                    m_wss[serverIP]=new WSClient(serverIP, QUrl(s), m_datapath);
                    connect(m_wss[serverIP], &WSClient::iperfStarted, this, &QIperfC::onIperfStarted);
                    connect(m_wss[serverIP], &WSClient::iperfStoped, this, &QIperfC::onIperfStoped);
                    connect(m_wss[serverIP], &WSClient::disconnected, this, &QIperfC::onDisconnected);
                    connect(m_wss[serverIP], &WSClient::iperfTPdata, m_tpmgr, &TPMgr::onIperfTPdata);
                }else{
                    m_wss[serverIP]->setDatapath(m_datapath);
                }
                itimeout = iTimeout;
                while (itimeout>0 && (bErrorStop==0)&& (bUserStop==false)){
                    QThread::msleep(10);
                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                    //m_wss.contains(serverIP) && ! m_wss[serverIP]->isConnected() &&
                    if (m_wss.contains(serverIP)){
                        if (m_wss.value(serverIP)->isConnected()){
                            break;
                        }
                    }else{
                        bErrorStop = 1;
                        tp->setComment("ERROR: "+serverIP+" not connected");
                        emit errorStop(2, "ERROR: "+serverIP+" not connected");
                        break;
                    }
                    itimeout--;
                    emit updateStatus(" wait WSClient connect to server: "+ s +
                                      " ("+ QString::number(itimeout) +")");
                }
                if (itimeout<=0){
                    emit errorStop(1,"ERROR: Wait connect to " +s+ " timeout");
                    break;
                }
                if(bErrorStop>0){
                    qDebug() << "Some error happen!!";
                    return;
                }
                if(bUserStop){
                    qDebug() << "User Stop on wait server websocket connected!!";
                    return;
                }
                //tell server add iperf server
                cmd = QString(CMD_IPERF_ADD)+":"+QString::number(refrow)+":"+tp->getServerArgs();
    //            qInfo() << "server cmd:"<< serverIP << " CMD_IPERF_ADD:" << tp->getServer() << ":" << tp->getPort() ;
                rs = m_wss[serverIP]->sendText(cmd);
                if (rs<=0){
                    emit errorStop(1, "Setup server iperf config fail: "+ tp->getServerArgs());
                    break;
                }
                m_status_server[tp->getBindKey(true)]=TPStatus::init; // init server of BindKey status 0
                //RPC to control all client endpoint (iperf client)
                QString clientIP = tp->getMgrClient();
                //TODO: detect manager client is pingable
                if (!m_wsc.contains(clientIP)) {
                    s = "ws://"+clientIP+":"+QString::number(QIPERFD_WSPORT);
                    m_wsc[clientIP]=new WSClient(clientIP, QUrl(s), m_datapath);
                    connect(m_wsc[clientIP], &WSClient::iperfStarted, this, &QIperfC::onIperfStarted);
                    connect(m_wsc[clientIP], &WSClient::iperfStoped, this, &QIperfC::onIperfStoped);
                    connect(m_wsc[clientIP], &WSClient::disconnected, this, &QIperfC::onDisconnected);
                    connect(m_wsc[clientIP], &WSClient::iperfTPdata, m_tpmgr, &TPMgr::onIperfTPdata);
                }else{
                    m_wsc[clientIP]->setDatapath(m_datapath);
                }
                itimeout = iTimeout;
                while (! m_wsc[clientIP]->isConnected()&& itimeout>0&& (bErrorStop==0)&& (bUserStop==false)){
                    QThread::msleep(10);
                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                    if (m_wsc.contains(clientIP)){
                        if (m_wsc.value(clientIP)->isConnected()){
                            break;
                        }
                    }else{
                        bErrorStop = 1;
                        tp->setComment("ERROR: "+clientIP+" not connected");
                        emit errorStop(2, "ERROR: "+clientIP+" not connected");
                        break;
                    }
                    itimeout--;
                    emit updateStatus(" wait WSClient connect to client: "+s);
                }
                if (itimeout<=0){
                    emit errorStop(1,"ERROR: Wait connect to " +s+ "timeout");
                    break;
                }
                if(bErrorStop>0){
                    qDebug() << "Some error happen!!";
                    return;
                }
                if(bUserStop){
                    qDebug() << "User Stop on wait client websocket connected!!";
                    return;
                }
                //tell client add iperf client
                cmd = QString(CMD_IPERF_ADD)+":"+QString::number(refrow)+":"+tp->getClientArgs();
    //            qInfo() << "client cmd:" << clientIP << " CMD_IPERF_ADD:" << tp->getClient() << ":" << tp->getPort();
                rs = m_wsc[clientIP]->sendText(cmd);
                if (rs<=0){
                    emit errorStop(2, "Setup client iperf config fail: "+ tp->getClientArgs());
                    break;
                }
                m_status_client[tp->getBindKey(false)]=TPStatus::init;// init client of BindKey status 0
            } else {
                qDebug() << "Ignore disabled TP test pair: " << tp;
            }

        }
        //TODO: record which should report iperf throughput value
        // TODO: list of ping test

        if(bErrorStop>0){
            qDebug() << "Some error happen!!";
            return;
        }
        //Start server
        for (auto key: m_wss.keys()){
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            rs = m_wss[key]->sendText(QString(CMD_IPERF_START)+":"+startTime);
            if (rs<=0){
                emit errorStop(3, "Start iperf server fail:" + key);
                break;
            }
        }
        if(bErrorStop>0){
            qDebug() << "Start server error happen!!";
            return;
        }
        //###############################
         QThread::sleep(3);
        //TODO: wait server start up and ready
        QDateTime oldDT = QDateTime::currentDateTime();
        QDateTime newDT;
        int iwaittime;
        int chk=0;
        bool bServerReady=false;
        while (!bServerReady && (bUserStop==false)){ //TODO: timeout!!!
            QThread::sleep(1);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            chk=0;
            foreach(auto skey, m_status_server.keys()){
                QCoreApplication::processEvents(QEventLoop::AllEvents);
                if (m_status_server.value(skey, 0)==TPStatus::started){
                    chk++;
                }
            }
            if (chk>=m_status_server.keys().length()){
                bServerReady=true;
            }
            newDT = QDateTime::currentDateTime();
            iwaittime = m_WaitServerReady - oldDT.secsTo(newDT);
            emit updateStatus(" wait server ready: "+ QString::number(chk)+ "/"+
                              QString::number(m_status_server.keys().length())+
                              ":"+ QString::number(iwaittime));

//            if (oldDT.secsTo(newDT)>m_WaitServerReady){
            if (iwaittime<0){
                // timeout
                break;
            }
        }
        if(bUserStop){
            qDebug() << "User Stop on wait ServerReady!!";
            return;
        }
        if (!bServerReady){
            QStringList ds;
            foreach (auto key, m_status_server.keys()){
                if (m_status_server[key]!=TPStatus::started){
                    ds.append(key);
                }
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
            qDebug() << "server not readey: " << ds.join(",");
            emit errorStop(4, "Iperf server not readey:" +  ds.join(","));
            return;
        }
        //Start client
        for (auto key: m_wsc.keys()){
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            rs = m_wsc[key]->sendText(QString(CMD_IPERF_START)+":"+startTime);
            if (rs<=0){
                emit errorStop(4, "Start iperf client fail:" + key);
                break;
            }
        }
        if(bErrorStop>0){
            qDebug() << "Start client error happen!!";
            return;
        }
        QDateTime waitStartTime = QDateTime::currentDateTime();
        QDateTime waitEndTime = QDateTime::currentDateTime();
        int iWait = waitStartTime.secsTo(waitEndTime);
        while ((iWait < maxtestduration) && (bUserStop==false)){
            if ((getStatusServers()>m_status_server.keys().length()) ||
                (getStatusClients()>m_status_client.keys().length())) {
                qDebug() << "Some problem happen!! abort!! server:" << m_status_server <<
                            " client:" << m_status_client;
                break;
            }else if(getStatusServers()==0 && getStatusClients()==0) {
                //qDebug() << "All test end, stop early";
                break;
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            QThread::msleep(100);
            emit updateStatus("Remain "+ QString::number(maxtestduration-iWait) + " sec");
            waitEndTime = QDateTime::currentDateTime();
            iWait = waitStartTime.secsTo(waitEndTime);
        }
        onStop();
    } else {
        QMessageBox::information(this,"NOTICE", "Plase add iperf test pair first!");
    }
}

void QIperfC::onStop(){
    QString cmd="";
    foreach (auto key, m_wsc.keys()){
        if (m_wsc[key]){
            cmd = QString(CMD_IPERF_STOP)+":" + key;
            qDebug() << m_wsc[key] << " m_wsc send cmd: " << cmd;
            m_wsc[key]->sendText(cmd);
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    foreach (auto key, m_wss.keys()){
        if (m_wss[key]){
            cmd = QString(CMD_IPERF_STOP)+":" + key;
            qDebug() << m_wss[key] <<  "m_wss send cmd: " << cmd;
            m_wss[key]->sendText(cmd);
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    bUserStop=true;
    if (m_fileserver->getSockets()>0){
        m_fileserver->close();
    }
    updateRunStatus(false);
    QString endtime = getNowString();
    QDateTime enddatetime = QDateTime::fromString(endtime,DATETIME_NOW_FORMAT);
    emit updateStatus("Finish at  "+ endtime +" (Runtime: "+QString::number(m_TestStartTime.secsTo(enddatetime))+" sec)");
}

bool QIperfC::onClear(){
    if (m_TestStartTime.isValid()){
        int ret = QMessageBox::information(this, "NOTICE", "Previous test record will be clear, Continious?", QMessageBox::Ok|QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel){
            // test cancel
            return false;
        }
    }
    doClear();
    return true;
}

void QIperfC::onShowLog()
{
    if (!m_datapath.isEmpty()){
        QDir d(m_datapath);
        if (d.exists()){
            m_dlgrecord->setRootPath(m_datapath);
            m_dlgrecord->show();
        }else{
            QMessageBox::information(this, "ERROR", "No test record folder: " + m_datapath);
        }
    }else{
        QMessageBox::information(this, "ERROR", "No test record");
    }
}

void QIperfC::onConfig()
{
    m_frm_option->setWaitServerReady(m_WaitServerReady);
    int rc = m_frm_option->exec();
    if (rc == QDialog::Accepted){
        //update setting
        m_WaitServerReady = m_frm_option->getWaitServerReady();
    }
}

void QIperfC::onCopy()
{
    if (ui->tv_throughput->hasFocus()){
        QModelIndexList idxs = ui->tv_throughput->selectionModel()->selectedRows();
//        qDebug() << "onCopy:" << idxs;
        TP *tp;
        QString s="";
        foreach(auto idx, idxs){
            tp = m_tpmgr->getItem(idx);
            s = s + "\n" + tp->getJsonData();
        }
        m_clipboard->setText(s);
    }else {
        qDebug() << "tv_throughput no hasFocus";
    }
}

void QIperfC::onPaste()
{
    if (ui->tv_throughput->hasFocus()){
        QString clip = m_clipboard->text();
        m_tpmgr->onPaste(clip);
    }else {
        qDebug() << "tv_throughput no hasFocus";
    }
}

void QIperfC::onDelete()
{
    onPairDelete();
}

void QIperfC::onAbout()
{
    QMessageBox::about(this, "About", QString(QIPERFC_NAME)+
                       " v"+QString(QIPERFC_VERSION)+"\n"
                       "Auther: Jimmy Yeh\n"
                                                     "URL: https://github.com/coolshou/qiperf");
}

void QIperfC::onShowDebugLog()
{
    m_dlgshowlog->open();
    m_dlgshowlog->raise();
    m_dlgshowlog->activateWindow();
}

void QIperfC::aboutQCustomPlot()
{
    QMessageBox::about(this, "About QCustomPlot", "QCustomPlot\n"
                       "Ver: "+ QString(QCUSTOMPLOT_VERSION_STR) + "\n"
                       "URL: https://www.qcustomplot.com/index.php/introduction");
}

void QIperfC::onErrorStop(int err, QString msg)
{
    bErrorStop = err;
    m_ErrorMSG = msg;
    qDebug() << "onErrorStop: (" <<bErrorStop <<") " << m_ErrorMSG;
    emit updateStarttime("");
    emit updateStatus(msg);
    updateRunStatus(false);
//    onStop();
}

void QIperfC::on_notice(QString send_addr, QString msg)
{
    //receive qiperfd notices
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
    // check validity of the document
//    if(!doc.isNull()) {
        QJsonObject obj = doc.object();
        int act = obj["ACT"].toInt();
        switch (act){
            case EndPointAct::Add:
//                qDebug() << "QIperfC::on_notice:" << send_addr << "\nmsg:" << msg;
                if (m_endpointmgr->add(send_addr, msg)){
                    if (dlgiperf){
//                        qInfo() << "on_notice:EndPointAct:Add: " << send_addr << " msg: " << msg;
//                        if (dlgiperf->add(send_addr)){
                        if (dlgiperf->add(send_addr, msg)){
                            dlgiperf->updateUI();
                        }
                        // TODO: ping dialog
                    }
                    emit updateEndpointNum(m_endpointmgr->getTotalEndpoints());
                }
                break;
            case EndPointAct::Update:
                qDebug() << "TODO qiperfd Update: from(" << send_addr << ") " << msg << Qt::endl;
                break;
            case EndPointAct::Del:
                qDebug() << "TODO qiperfd Del: from(" << send_addr << ") " << msg << Qt::endl;
                break;
            case EndPointAct::Disable:
                qDebug() << "TODO qiperfd Disable: from(" << send_addr << ") " << msg << Qt::endl;
                break;
        }
    } else {
        qDebug() << "TODO on_notice invalid message: from(" << send_addr << ") " << msg;
    }
}

void QIperfC::onQuit()
{
    qInfo() << "onQuit" << Qt::endl;
    // TODO: do any thing before quit!
    qApp->quit();
}

void QIperfC::notificationReceived(const QString key, const QVariant value)
{
    qDebug() << "RPC Received notification:"
                     << "Key:" << key
                     << "Value:" << value << Qt::endl;
}

void QIperfC::setStartTime(QDateTime startTime)
{
    // qDebug() << "QIperfC::setStartTime: " << startTime.toString(DATETIME_NOW_FORMAT);
    m_TestStartTime = startTime;
}

void QIperfC::onTest()
{
    QString strJson;
#if (TEST_ICMP==1)
    if (dp->exec()== QDialog::Accepted){
        strJson = dp->getJsonstr();
        qDebug() << "strJson:" << strJson;
        m_icmpping = new IcmpPing("0", strJson, nullptr);
    } else{
        m_icmpping = new IcmpPing("0", "192.168.0.1", 10, 3, 1, 64, "192.168.0.47", 64, nullptr);
    }
//    connect(m_icmpping, &IcmpPing::icmpResponseTime, this, );
    m_icmpping->start();
#endif
    if (0){
        // test remote ping

        QString s = "ws://192.168.70.147:"+QString::number(QIPERFD_WSPORT);
        ws= new WSClient("192.168.70.147", QUrl(s), m_datapath);
        QString cmd = QString(CMD_PING)+":"+QString::number(0)+":"+strJson;
        qDebug() << "onTest cmd:" << cmd;
        ws->sendText(cmd);
    }
}

void QIperfC::closeEvent(QCloseEvent *event)
{
    //TODO: check config edit.
    Q_UNUSED(event);

    saveSettings();
    emit closeAll();
}

bool QIperfC::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == m_label_qiperfd && event->type() == QMouseEvent::MouseButtonPress) {
//        m_frm_qiperfds->setGeometry();
        int dx = m_frm_qiperfds->geometry().width()/2;
        int dy = m_frm_qiperfds->geometry().height()/2;
        QPoint p(this->geometry().center().x()-dx, this->geometry().center().y()-dy);
        m_frm_qiperfds->move(p);
        m_frm_qiperfds->show();
        m_frm_qiperfds->raise();
        m_frm_qiperfds->activateWindow();
    }
//    if((obj == ui->menubar || obj == ui->toolBar) &&
//            (event->type() == (Qt::Key_Control & QMouseEvent::MouseButtonPress))) {
//        qDebug() << "show menuTest";
//        ui->menuTest->setVisible(true);
//        ui->menuTest->setEnabled(true);

//    }

    return QObject::eventFilter(obj,event);
}
void QIperfC::updateRunStatus(bool bStart)
{
    ui->actionStart->setEnabled(!bStart);
    ui->actionStop->setEnabled(bStart);
    ui->actionClear->setEnabled(!bStart);
}

void QIperfC::initThroughputChart()
{
    // throughput chart
    m_tpplot=new TPPlot(ui->widget_console);
    m_tpplot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tpplot, &TPPlot::customContextMenuRequested, this, &QIperfC::onPlotContextMenuRequest);
    ui->hl_console->addWidget(m_tpplot);

    m_tpmgr = new TPMgr(this);
    connect(m_tpmgr, &TPMgr::rowsInserted, this, &QIperfC::onTPDataUpdate);
    connect(m_tpmgr, &TPMgr::rowsRemoved, this, &QIperfC::onTPDataUpdate);
    connect(m_tpmgr, &TPMgr::IperfTPdata, m_tpplot, &TPPlot::onIperfTPdata);

    ui->tv_throughput->setModel(m_tpmgr);
    /* TODO: set specify column font size,
    // current not inherent other setting
    header = new CustomHeaderView(Qt::Horizontal, ui->tv_throughput);
    int s = header->getFontSize();
    header->setColumnSize(int(TP::mintp), s/2);
    header->setColumnSize(int(TP::maxtp), s/2);
    ui->tv_throughput->setHeader(header);
    // set specify column font size
//    QHeaderView *header = ui->tv_throughput->header();
//    QFont font = header->font();
//    qDebug() << "font size: " << font.pointSize();
//    font.setPointSize(28); // Set the desired font size
//    header->setStyleSheet(QString("QHeaderView::section:nth-child(%1) { font-size: %2pt; }").arg(1).arg(font.pointSize()));
    // end set font size

    */
    ui->tv_throughput->setColumnWidth(TP::cols::id, 100);
    ui->tv_throughput->setColumnWidth(TP::cols::server, 180);
    ui->tv_throughput->setColumnWidth(TP::cols::dir, 80);
    ui->tv_throughput->setColumnWidth(TP::cols::client, 180);
    ui->tv_throughput->setColumnWidth(TP::cols::lostrate, 110);

    TooltipEventFilter* filter = new TooltipEventFilter(ui->tv_throughput);
    connect(filter, &TooltipEventFilter::doCopy, this, &QIperfC::onCopy);
    connect(filter, &TooltipEventFilter::doPaste, this, &QIperfC::onPaste);
    connect(filter, &TooltipEventFilter::doDelete, this, &QIperfC::onDelete);
    ui->tv_throughput->viewport()->installEventFilter(filter);
    ui->tv_throughput->setRootIsDecorated(true); //show folding icon
//    ui->tv_throughput->setRootIndex(m_tpmgr->getRootItemIdx());
//    ui->tv_throughput->expand(m_tpmgr->getRootItemIdx());
    ui->tv_throughput->expandAll();// will show folding icon when have child item??
    ui->tv_throughput->setContextMenuPolicy(Qt::CustomContextMenu);  // custom right click menu
    connect(ui->tv_throughput, &QTreeView::customContextMenuRequested, this, &QIperfC::onTPUTContextMenu);
    connect(ui->tv_throughput, &QTreeView::doubleClicked, this, &QIperfC::onItemDClicked); //edit item on double click

    //TODO: slow update text/image?
    tpdirdelegate = new TPDirDelegate(ui->tv_throughput);
//    tpdirdelegate = new TPDirDelegate(this);
    ui->tv_throughput->setItemDelegateForColumn(TP::cols::dir, tpdirdelegate);
    // TODO: why debug build do not show folding icon!!
//    tpfoldingdelegate = new TPFoldingDelegate(ui->tv_throughput);
//    ui->tv_throughput->setItemDelegateForColumn(TP::cols::id, tpfoldingdelegate);

    QItemSelectionModel *ism = ui->tv_throughput->selectionModel();
    connect(ism, &QItemSelectionModel::selectionChanged, this, &QIperfC::onTPselectionChanged);
//    ui->tv_throughput->header()->setVisible(true);
}

void QIperfC::initPingChart()
{
    if (!m_testping){
//        ui->tab_ping->setVisible(false);
        ui->tabwidget->setTabVisible(1, false);
    }
    m_pingmgr = new PingMgr();
    // ping chart
    m_pingplot = new PingPlot(ui->widget_ping);
    ui->hl_ping->addWidget(m_pingplot);
}

void QIperfC::resetError()
{
    bErrorStop = 0;
    m_ErrorMSG = "";
}

void QIperfC::saveSettings()
{
    m_settings->beginGroup("MainWindow");
    m_settings->setValue("geometry", saveGeometry());
    m_settings->setValue("windowState", saveState());
    m_settings->setValue("oldsavepath", m_oldsavepath);
    m_settings->sync(); // forces to write the settings to storage
    m_settings->endGroup();
    m_settings->beginGroup("Iperf");
    m_settings->setValue("WaitServerReady", m_WaitServerReady);
    m_settings->setValue("TPExportWidth", m_TPExportWidth);
    m_settings->setValue("TPExportHeigth", m_TPExportHeigth);
    m_settings->endGroup();
    m_settings->sync();
}

void QIperfC::loadSettings()
{
    m_settings->beginGroup("MainWindow");
    // default to screen center
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    int x = (screen.width()-rect().width())/2;
    int y = (screen.height()-rect().height())/2;
    QRect newrect = QRect(x, y, rect().width(), rect().height());
    move(x,y);
    restoreGeometry(m_settings->value("geometry", newrect).toByteArray());
    restoreState(m_settings->value("windowState").toByteArray());
    m_oldsavepath = m_settings->value("oldsavepath",
                                      QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    m_settings->endGroup();
    m_settings->beginGroup("Iperf");
    m_WaitServerReady =m_settings->value("WaitServerReady", 10).toInt();
    m_TPExportWidth =m_settings->value("TPExportWidth", 1280).toInt();
    m_TPExportHeigth =m_settings->value("TPExportHeigth", 180).toInt();
//    m_frm_option->setWaitServerReady();
    m_settings->endGroup();
    m_settings->beginGroup("test");
    m_testping = m_settings->value("testping", false).toBool();
    m_settings->endGroup();
}

void QIperfC::doClear()
{
    //clear all test date, config setting remain unchanged
    if (m_tpmgr->rootChildCount()>0) {
        m_tpmgr->clear();
        ui->tv_throughput->collapseAll();
    }
    m_tpplot->clear();
    m_TestStartTime = QDateTime();
    m_qipconfig->clear();
    emit updateStatus("");
    emit updateStarttime("");
    ui->actionShowLog->setEnabled(false);
}

void QIperfC::onExport()
{
    if (m_TestStartTime.isValid()){
        //export test record to html file
        QString templatefile = qApp->applicationDirPath()+QDir::separator()+"template"+QDir::separator()+"result.html";

        QString path;
        if (!m_oldsavepath.isNull()){
            path = m_oldsavepath;
        }else {
            path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        }
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Export Result to html"), path, tr(HTML_EXT_FILTER));
        QFileInfo fi(fileName);
        // QString img = fi.path() +QDir::separator()+ fi.baseName()+".png";
        QString ext = fi.suffix();
        if (ext.compare(HTML_EXT)!=0){
            fileName = fi.path() +QDir::separator()+ fi.baseName() + "."+ HTML_EXT;
        }

        ExportHtml *eh = new ExportHtml(templatefile, fileName, m_TPExportWidth, m_TPExportHeigth);
        eh->setData(m_tpmgr, m_tpplot);
        // QThread::sleep(1);//TODO: any better way to wait page loaded??
        // eh->setData(m_tpmgr, m_tpplot);

        // eh->save(fileName);

        // //prepare throughput config data
        // if (m_tpmgr->rootChildCount()>0) {
        //     QByteArray b = m_tpmgr->savedata();
        //     QStringList pcs = m_tpmgr->getPCs();
        //     QString env= m_endpointmgr->getPCsInfo(pcs);
        //     //        qDebug() << "env: " << env;
        //     QString starttime = m_TestStartTime.toString(DATETIME_NOW_FORMAT);
        //     QString tmp = m_logpath + QDir::separator() + starttime;
        //     QDir d(tmp);
        //     QStringList filelist;
        //     foreach(auto s, d.entryList(QDir::Files)){
        //         filelist.append(tmp+ QDir::separator()+s);
        //     }
        //     m_qipconfig->setTPCfg(b, env, starttime, filelist);
        //     if (!m_qipconfig->exportToFile(fileName, m_tpplot)){
        //         qDebug() << "Export to " << fileName << " Fail!";
        //     }
        // }else{
        //     qDebug() << "NO throughput config to save";
        // }
    }else {
        qDebug() << "NO throughput record to Export : TestStartTime: " << m_TestStartTime.toString(DATETIME_NOW_FORMAT);
    }
}

void QIperfC::initMenus()
{
    m_tpmenu = new QMenu();
    m_aEnable = new QAction("Enable select item");
    connect(m_aEnable, &QAction::triggered, this, &QIperfC::onEnableItem);
    m_aDisable = new QAction("Disable select item");
    connect(m_aDisable, &QAction::triggered, this, &QIperfC::onDisableItem);
//    aDisable->setEnabled(false);
    m_tpmenu->addAction(ui->actionCopy);
    m_tpmenu->addAction(ui->actionPaste);
    m_tpmenu->addAction(ui->actionDelete);
    m_tpmenu->addSeparator();
    m_tpmenu->addAction(m_aEnable);
    m_tpmenu->addAction(m_aDisable);

}

void QIperfC::onRPC_result(const QVariant &result)
{
    qDebug() << "onRPC_result: " << result << Qt::endl;
}

void QIperfC::onRPC_error(int code, const QString &message)
{
    qDebug() << "onRPC_error: (" << code << ")" << message << Qt::endl;
}

void QIperfC::onIperfStarted(QString smode, QString ipport)
{
    qDebug() << "onIperfStarted:" << smode << " : " << ipport;
    if (smode.contains("S", Qt::CaseSensitive)){
        m_status_server[ipport]=TPStatus::started;
    }else{
        m_status_client[ipport]=TPStatus::started;
    }
}

void QIperfC::onIperfStoped(QString refrow, QString err_no, QString err, QString ipport)
{
    qDebug() << "onIperfStoped:" << refrow << " : " << ipport <<
        " err_no:" << err_no << " err:" << err;
    if (err_no.toInt()>0){
        m_tpmgr->addComment(refrow, "["+ ipport +"]Error:" +err);
        if (m_status_server.contains(ipport)){
            m_status_server[ipport]=TPStatus::stoped;
        }
        if (m_status_client.contains(ipport)){
            m_status_client[ipport]=TPStatus::stoped;
        }
//        emit errorStop(2, "onIperfStoped: ["+ipport+"]("+err_no+"):"+err);
    }else{

        // qiperf notify no error end:
        if (m_status_server.contains(ipport)){
            // qDebug() << "m_status_server[" << ipport << "]: " << m_status_server[ipport];
            m_status_server[ipport]=TPStatus::init;
        }
        if (m_status_client.contains(ipport)){
            // qDebug() << "m_status_client[" << ipport << "]: " << m_status_client[ipport];
            m_status_client[ipport]=TPStatus::init;
        }
    }
}

void QIperfC::onDisconnected(QString targetip)
{
    qDebug() << "onDisconnected: " << targetip;
    if (m_wss.contains(targetip)){
        m_wss.remove(targetip);
    }
    if (m_wsc.contains(targetip)){
        m_wsc.remove(targetip);
    }
}

void QIperfC::onTPUTContextMenu(QPoint pos)
{
    // if select multi items
    QModelIndexList idxs = ui->tv_throughput->selectionModel()->selectedRows();
    if (idxs.length()>1){
        m_aEnable->setEnabled(true);
        m_aDisable->setEnabled(true);
    } else {
        //TODO: check select item status enable/disable menu item
        QModelIndex midx = ui->tv_throughput->indexAt(pos);
        TP *tp = m_tpmgr->getItem(midx);
        if (tp->getEnabled()){
            m_aEnable->setEnabled(false);
            m_aDisable->setEnabled(true);
        }else{
            m_aEnable->setEnabled(true);
            m_aDisable->setEnabled(false);
        }
    }
    m_tpmenu->popup(ui->tv_throughput->mapToGlobal(pos));
}

void QIperfC::onPlotContextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
//    if (ui->customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
//    {
//        menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignLeft));
//        menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignHCenter));
//        menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignRight));
//        menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignRight));
//        menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignLeft));
//    }
//    else  // general context menu on graphs requested
    {
//        menu->addAction("Add random graph", this, SLOT(addRandomGraph()));
//        if (ui->customPlot->selectedGraphs().size() > 0)
//            menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
//        if (ui->customPlot->graphCount() > 0)
//            menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
        menu->addAction("About", this, &QIperfC::aboutQCustomPlot);
    }

    menu->popup(m_tpplot->mapToGlobal(pos));

}

void QIperfC::onUpdateDataPath(QString datapath)
{
    m_datapath = datapath;
    m_dlgrecord->setRootPath(datapath);
    ui->actionShowLog->setEnabled(true);
}

void QIperfC::onUpdateTPCfg(QByteArray tpcfg)
{
    m_tpmgr->loaddata(tpcfg);
}

void QIperfC::onProgress(int currentlineno)
{
    if (currentlineno>0){
        onUpdateStatus("Procress line "+ QString::number(currentlineno));
    }else{
        onUpdateStatus("");
    }
}

int QIperfC::getStatusServers()
{
    int sum = 0;
    for (auto value : m_status_server) {
        sum += value;
    }
    return sum;
}

int QIperfC::getStatusClients()
{
    int sum = 0;
    for (auto value : m_status_client) {
        sum += value;
    }
    return sum;
}

void QIperfC::onEnableItem(bool checked)
{
    Q_UNUSED(checked)
//     ui->tv_throughput->SelectItems;
    QModelIndexList idxs = ui->tv_throughput->selectionModel()->selectedIndexes();
    if (idxs.length()>0){
        TP *tp;
        QString s="";
        foreach(auto idx, idxs){
            tp = m_tpmgr->getItem(idx);
            tp->setEnabled();
        }
    }

}

void QIperfC::onDisableItem(bool checked)
{
    Q_UNUSED(checked)
    QModelIndexList idxs = ui->tv_throughput->selectionModel()->selectedRows();
    if (idxs.length()>0){
        TP *tp;
        QString s="";

        foreach(auto idx, idxs){
            tp = m_tpmgr->getItem(idx);
            tp->setDisabled();
        }
    }
}

void QIperfC::initActions()
{
    // init actions
    // file
    connect(ui->actionNew, &QAction::triggered, this, &QIperfC::onNew);
    connect(ui->actionOpen, &QAction::triggered, this, &QIperfC::onOpen);
    connect(ui->actionSave, &QAction::triggered, this, &QIperfC::onSave);
    ui->actionSave->setEnabled(false);
    connect(ui->actionIperf3Log, &QAction::triggered, this, &QIperfC::onImportIperf3Log);
    connect(ui->actionExport, &QAction::triggered, this, &QIperfC::onExport);

    // edit
    connect(ui->actionCopy, &QAction::triggered, this, &QIperfC::onCopy);
    connect(ui->actionPaste, &QAction::triggered, this, &QIperfC::onPaste);

    connect(ui->actionAddIperf, &QAction::triggered, this, &QIperfC::onAddIperf);
    if (!m_testping){
        ui->actionAddPing->setVisible(false);
    }
    connect(ui->actionAddPing, &QAction::triggered, this, &QIperfC::onAddPing);

    connect(ui->actionEdit, &QAction::triggered, this, &QIperfC::onPairEdit);
    connect(ui->actionDelete, &QAction::triggered, this, &QIperfC::onPairDelete);
    connect(ui->actionSwap, &QAction::triggered, this, &QIperfC::onPairSwap);
    connect(ui->actionSwapIP, &QAction::triggered, this, &QIperfC::onPairSwapIP);

    // run
    connect(ui->actionStart, &QAction::triggered, this, &QIperfC::onStart);
    connect(ui->actionStop, &QAction::triggered, this, &QIperfC::onStop);
    connect(ui->actionClear, &QAction::triggered, this, &QIperfC::onClear);
    connect(ui->actionShowLog, &QAction::triggered, this, &QIperfC::onShowLog);

    //option
    connect(ui->actionConfig, &QAction::triggered, this, &QIperfC::onConfig);

    //help
    connect(ui->actionAbout, &QAction::triggered, this, &QIperfC::onAbout);
    connect(ui->actionShowDebugLog, &QAction::triggered, this, &QIperfC::onShowDebugLog);

    //test
    connect(ui->actionTest, &QAction::triggered, this, &QIperfC::onTest);

}

void QIperfC::initToolbar()
{
    QMenu *menuAdd = new QMenu(this);
    menuAdd->addAction(ui->actionAddIperf);
    menuAdd->addAction(ui->actionAddPing);

    ui->actionAdd->setMenu(menuAdd);

    ui->toolBar->insertAction(ui->actionEdit, ui->actionAdd);
}

void QIperfC::initStatusbar()
{
    m_start_label = new QLabel();
    m_start_label->setFrameStyle(static_cast<int>(QFrame::StyledPanel) | static_cast<int>(QFrame::Sunken));
    ui->statusbar->addWidget(m_start_label, 1);
    connect(this , &QIperfC::updateStarttime, this,  &QIperfC::onUpdateStarttime);

    m_status_label = new QLabel();
    m_status_label->setFrameStyle(static_cast<int>(QFrame::StyledPanel) | static_cast<int>(QFrame::Sunken));
    ui->statusbar->addWidget(m_status_label, 2);
    connect(this , &QIperfC::updateStatus, this,  &QIperfC::onUpdateStatus);

    // statusbar of endpints
    m_label_qiperfd = new QLabel(this);
    m_label_qiperfd->installEventFilter(this);
//TODO: double click
    m_label_qiperfd->setText(QString(QIPERFD_NAME)+":0");
    m_label_qiperfd->setFrameStyle(static_cast<int>(QFrame::Box) | static_cast<int>(QFrame::Sunken));
//    m_endpoint_label->setTextFormat(Qt::RichText);
//    m_endpoint_label->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(m_label_qiperfd);
    connect(this , &QIperfC::updateEndpointNum, this, &QIperfC::on_updateQIperfdNum);
}

void QIperfC::onUpdateStarttime(QString stime)
{
    m_start_label->setText(stime);
}

void QIperfC::onUpdateStatus(QString msg)
{
    m_status_label->setText(msg);
}

void QIperfC::on_updateQIperfdNum(int n)
{
    m_label_qiperfd->setText(QString(QIPERFD_NAME)+ ":" + QString::number(n));
}

void QIperfC::onTPselectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

    bool bAct=false;
    if (selected.length()>0){
        bAct = true;
    }
    ui->actionDelete->setEnabled(bAct);
    ui->actionEdit->setEnabled(bAct);
    ui->actionSwap->setEnabled(bAct);
    ui->actionSwapIP->setEnabled(bAct);
}

void QIperfC::onTPDataUpdate(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    //TODO: when throughput is running, add new TP item?
    bool bStart;
    if (m_tpmgr->rowCount()>0) {
        bStart=false;
        ui->actionStart->setEnabled(!bStart);
        ui->actionStop->setEnabled(bStart);
        ui->actionClear->setEnabled(!bStart);
    } else {
        ui->actionStart->setEnabled(false);
        ui->actionStop->setEnabled(false);
//        ui->actionClear->setEnabled(false);
    }
    //    updateRunStatus(bStart);
}

void QIperfC::onItemDClicked(QModelIndex idx)
{
    TP *tp = m_tpmgr->getItem(idx);
    if (tp->getDataType() == TPMgrData::config) {
        // only iperf pair config can be edit
        dlgiperf->loadJsonCfg(tp->saveData());
        dlgiperf->setExcIdx(idx);
        int rc = dlgiperf->exec();// show dlgiperf
        if (rc == QDialog::Accepted){
            QString rs= dlgiperf->getJsonCfg();
            tp->loadData(rs);
            m_tpmgr->setItem(idx, tp);
        }
    }
}
