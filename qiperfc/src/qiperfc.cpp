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
//    ui->menubar->installEventFilter(this);
//    ui->menuTest->setVisible(false);
//    ui->menuTest->setEnabled(false);
//    ui->toolBar->installEventFilter(this);
    m_dlgtest = new DlgTest();
    m_frm_qiperfds = new FormQIperfds();
    m_frm_option = new dlgOption(m_settings);
    initStatusbar();
    loadSettings();
    //UI actions
    initActions();
    initToolbar();
    //dataTimer = QTimer();
    initCustomPlote();
    connect(this, &QIperfC::errorStop, this, &QIperfC::onErrorStop);

    iTimeout = 10*100;
    //
    m_tpmgr = new TPMgr(this);
    connect(m_tpmgr, &TPMgr::rowsInserted, this, &QIperfC::onTPDataUpdate);
    connect(m_tpmgr, &TPMgr::rowsRemoved, this, &QIperfC::onTPDataUpdate);
    connect(m_tpmgr, &TPMgr::IperfTPdata, m_tpplot, &TPPlot::onIperfTPdata);

    m_qipconfig = new QIPConfig(logdir.absolutePath());
    connect(m_qipconfig, &QIPConfig::updateDataPath, this, &QIperfC::onUpdateDataPath);
    connect(m_qipconfig, &QIPConfig::updateTPCfg, this, &QIperfC::onUpdateTPCfg);
    connect(m_qipconfig, &QIPConfig::onThroughput, m_tpmgr, &TPMgr::onIperfTPdata);

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

    m_endpointmgr = new EndPointMgr(this);
    m_frm_qiperfds->setModel(m_endpointmgr);

    dlgiperf = new DlgIperf(m_tpmgr, this);

    //
    m_receiver = new UdpReceiver(QIPERFD_BPORT,this);
    connect(m_receiver, &UdpReceiver::notice, this, &QIperfC::on_notice);

    // control local qiperfd?
    pclient = new PipeClient(QIPERFD_NAME);
    connect(pclient, SIGNAL(newMessage(QString)), this, SLOT(onNewMessage(QString)));
    pclient->SetAppHandle(qApp);

    m_dlgrecord = new DlgRecord(this);
    m_fileserver = new FileServer(QIPERF_FILEPORT);

    // control remote qiperfd?
#if (TEST_JSONRPC==1)
    qDebug() << "test jcon rpc server" << Qt::endl;
    rpc_client = new jcon::JsonRpcWebSocketClient(parent);
    rpc_client->connectToServer("127.0.0.1", RPC_PORT);
    auto req = rpc_client->callAsync("getOS");
    req->connect(req.get(), &jcon::JsonRpcRequest::result,
                 [](const QVariant& result) {
                     qDebug() << "result of RPC call:" << result << Qt::endl;
                     qApp->exit();
                 });
    req->connect(req.get(), &jcon::JsonRpcRequest::error,
                 [](int code, const QString& message, const QVariant& data) {
                     qDebug() << "RPC error: " << message << " (" << code << ")" << data << Qt::endl;
                     qApp->exit();
                 });
//    if (result->isSuccess()) {
//        qDebug() << "OS: " << result->result() << Qt::endl;
//    }

#endif

}

QIperfC::~QIperfC()
{
//    qDebug() << "~QIperfC";
    delete ui;
}

bool QIperfC::load(QString filename)
{
    //load test config file
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
        //QByteArray b = m_qipconfig->getTPCfg();
        //m_tpmgr->loaddata(b);
        //TODO: load record to plot

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
//        m_qipconfig->setTPCfg(b );
        QString starttime="";
        if (m_TestStartTime.isValid()){
            starttime = m_TestStartTime.toString(DATETIME_NOW_FORMAT);
            QString tmp = m_logpath + QDir::separator() + starttime;
            QDir d(tmp);
//            qDebug() << "path: " << tmp << " :files: " << d.entryList(QDir::Files);
            QStringList filelist;
            foreach(auto s, d.entryList(QDir::Files)){
//                qDebug() << "save file: " << tmp+ QDir::separator()+s;
                filelist.append(tmp+ QDir::separator()+s);
            }
            m_qipconfig->setTPCfg(b, env, starttime, filelist);
        }else{
            m_qipconfig->setTPCfg(b, env);
        }
        m_qipconfig->saveToFile(filename);
        return true;
    }else {
        qDebug() << "NO throughput config to save" << Qt::endl;
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
    //TODO: check tp config exist?
    on_Clear();
}

void QIperfC::onOpen()
{
    //TODO: load test config file
    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open QIperf file"),
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
            tr(QIPERF_EXT_FILTER));
    QFileInfo fi(fileName);
    QString ext = fi.suffix();
    if (ext.compare(QIPERF_EXT)!=0){
        qDebug() << "Not support file format: " << fileName;
        return;
    }
    load(fileName);
}

void QIperfC::onSave()
{
    //TODO: save test config file
    QString fileName = QFileDialog::getSaveFileName(this,
             tr("Save QIperf "),
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
            tr(QIPERF_EXT_FILTER));
    //TODO: zip/tar file to save all data include throughput result...
    QFileInfo fi(fileName);
    QString ext = fi.suffix();
    if (ext.compare(QIPERF_EXT)!=0){
//        fileName = fileName + QIPERF_EXT
        fileName = fi.path() + fi.baseName() + "."+ QIPERF_EXT;
    }
    qInfo() << "save file: " << fileName ;
    save(fileName);

}

void QIperfC::on_Clear()
{
    if (m_tpmgr->rootChildCount()>0) {
        //this will clear all item include root!!
        m_tpmgr->reset();
    }else{
        qDebug() << "on_Clear No child";
    }
    onClear();
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
    }

}

void QIperfC::onPairAdd()
{
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
    if (!m_tpmgr->removeRow(cur.row(), cur.parent())){
        QMessageBox::information(this, "ERROR", "Can not remove test pair: " + cur.data().toString());
    }
}

void QIperfC::onPairSwap()
{
    QModelIndexList mls= ui->tv_throughput->selectionModel()->selectedRows();
    foreach (QModelIndex midx, mls) {
        m_tpmgr->swapDirection(midx);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
//    ui->tv_throughput->update();
}

void QIperfC::onStart()
{
    ui->actionShowLog->setEnabled(true);
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
    m_fileserver->setRootPath(m_datapath);

    emit updateStarttime(startTime);
    //if (m_tpmgr->children().count()>0) {
    if (m_tpmgr->rootChildCount()>0) {
        updateRunStatus(true);
        //start test
        QList<TP *> tps = m_tpmgr->getChilds();
        QString s;
        QString cmd;
        qint64 rs=0;
        int maxtestduration=0;
        int iwait=0;
        int itimeout;
        int refrow;
        foreach (TP *tp, tps) {
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            //RPC to control all server endpoint (iperf server)
            iwait = tp->getWaitTime();
            if (iwait> maxtestduration){
                maxtestduration = iwait;
            }
            refrow = tp->row();
            QString serverIP = tp->getMgrServer();
            //TODO: detect manager server is pingable
            if (!m_wss.contains(serverIP)) {
                s = "ws://"+serverIP+":"+QString::number(QIPERFD_WSPORT);
//                qDebug() << "server websocket url: " << s << Qt::endl;
                m_wss[serverIP]=new WSClient(serverIP, QUrl(s), m_datapath);
                connect(m_wss[serverIP], &WSClient::iperfStarted, this, &QIperfC::onIperfStarted);
                connect(m_wss[serverIP], &WSClient::iperfStoped, this, &QIperfC::onIperfStoped);
                connect(m_wss[serverIP], &WSClient::disconnected, this, &QIperfC::onDisconnected);
//                connect(m_wss[serverIP], &WSClient::iperfTPdata, this, &QIperfC::onIperfTPdata);
                connect(m_wss[serverIP], &WSClient::iperfTPdata, m_tpmgr, &TPMgr::onIperfTPdata);
                itimeout = iTimeout;
                while (! m_wss[serverIP]->isConnected() && itimeout>0){
                    QThread::msleep(10);
                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                    itimeout--;
                }
                if (itimeout<=0){
                    emit errorStop(1,"Wait connect to " +s+ " timeout");
                    break;
                }
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
//                connect(m_wsc[clientIP], &WSClient::iperfTPdata, this, &QIperfC::onIperfTPdata);
                connect(m_wsc[clientIP], &WSClient::iperfTPdata, m_tpmgr, &TPMgr::onIperfTPdata);
                itimeout = iTimeout;
                while (! m_wsc[clientIP]->isConnected()&& itimeout>0){
                    QThread::msleep(10);
                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                    itimeout--;
                }
                if (itimeout<=0){
                    emit errorStop(1,"Wait connect to " +s+ "timeout");
                    break;
                }
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

            // TODO. set websocket to  report throughput
//            QString di = tp->getDirection();
//            if (di== QVariant::fromValue(TP::DirType::Tx).toString()){
//                m_wss[serverIP]->sendText(QString(CMD_IPERF_REG)+":"+startTime+":Tx:"+tp->getBindKey(true));
//                m_wsc[clientIP]->sendText(QString(CMD_IPERF_UNREG)+":"+startTime+":"+tp->getBindKey(false));
//            }else if (di== QVariant::fromValue(TP::DirType::Rx).toString()){
//                m_wss[serverIP]->sendText(QString(CMD_IPERF_UNREG)+":"+startTime+":"+tp->getBindKey(true));
//                m_wsc[clientIP]->sendText(QString(CMD_IPERF_REG)+":"+startTime+":Rx:"+tp->getBindKey(false));
//            }else if (di== QVariant::fromValue(TP::DirType::TR).toString()){
//                m_wss[serverIP]->sendText(QString(CMD_IPERF_REG)+":"+startTime+":Tx:"+tp->getBindKey(true));
//                m_wsc[clientIP]->sendText(QString(CMD_IPERF_REG)+":"+startTime+":Rx:"+tp->getBindKey(false));
//            }else {
//                m_wss[serverIP]->sendText(QString(CMD_IPERF_REG)+":"+startTime+":Rx:"+tp->getBindKey(true));
//                m_wsc[clientIP]->sendText(QString(CMD_IPERF_REG)+":"+startTime+":Tx:"+tp->getBindKey(false));
//            }
        }
        //TODO: record which should report iperf throughput value

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
        while (!bServerReady){ //TODO: timeout!!!
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
            iwaittime = m_WaitServerReady-oldDT.secsTo(newDT);
            emit updateStatus(" wait server ready: "+ QString::number(chk)+ "/"+
                              QString::number(m_status_server.keys().length())+
                              ":"+ QString::number(iwaittime));

//            if (oldDT.secsTo(newDT)>m_WaitServerReady){
            if (iwaittime<0){
                // timeout
                break;
            }
        }
        if (!bServerReady){
            QStringList ds;
            foreach (auto key, m_status_server.keys()){
                if (m_status_server[key]!=TPStatus::started){
                    ds.append(key);
                }
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

        //TODO: wait all test done!!
        while (maxtestduration>0){
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            QThread::msleep(1000);
            maxtestduration --;
            emit updateStatus("Remain "+ QString::number(maxtestduration) + " sec");
        }
        //TODO: check all test done!!

        //clear all websocket
//        for (auto key: m_wss.keys()){
//            QCoreApplication::processEvents(QEventLoop::AllEvents);
//            rs = m_wss[key]->sendText(CMD_IPERF_CLEAR);
//            if (rs<=0){
//                emit errorStop(4, "clear iperf server config:" + key);
////                qDebug() << "rs: " << rs << " key:" << key;
//            }
//            m_wss.remove(key);
//        }
//        for (auto key: m_wsc.keys()){
//            QCoreApplication::processEvents(QEventLoop::AllEvents);
//            rs = m_wsc[key]->sendText(CMD_IPERF_CLEAR);
//            if (rs<=0){
//                emit errorStop(4, "clear iperf client config:" + key);
////                qDebug() << "rs: " << rs << " key:" << key;
//            }
//            m_wsc.remove(key);
//        }

        onStop();

#if (TEST_JSONRPC==1)
        //create RPC list for ipserf server and client
        foreach (TP *tp, tps) {
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            //TODO: check client ping server first
            //RPC to control all server endpoint init iperf server
            if (createRPC_Server(tp, tp->getMgrServer())==-1){
                emit errorStop(-1, "createRPC_Server at " + tp->getMgrServer() + " fail");
                return;
            }
            //RPC to control all client endpoint init iperf client
            if (createRPC_Client(tp, tp->getMgrClient())==-1){
                emit errorStop(-1, "createRPC_Client at " + tp->getMgrClient()+ " fail");
                return;
            }

        }

        foreach (QString mhost, map_qiperfds_server.keys()) {
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            //TODO: wait server ready/start
            qDebug() <<" Start iperf server: "<< mhost << Qt::endl;
            auto rpc_tp = map_qiperfds_server.value(mhost);
            // Add Iperf server
            auto result = rpc_tp->rpc->callNamedParams("addIperfServer",
                                        QVariantMap{{"refrow",0},
                                                    {"version",rpc_tp->tp->getVersion()},
                                                    {"port",rpc_tp->tp->getPort()},
                                                    {"bindHost", rpc_tp->tp->getServer()}});
            if (!result->isSuccess()){
                emit errorStop(-1, "addIperfServer at " + mhost +
                               " with " +rpc_tp->tp->getServer()+ ":" + rpc_tp->tp->getPort() +
                               " fail");
                return;
            }
            //start the Iperf server,
            auto req = rpc_tp->rpc->callAsyncNamedParams("start", QVariantMap{{"idx", result->result()}});
            req->connect(req.get(), &jcon::JsonRpcRequest::result, this, &QIperfC::onRPC_result);
            req->connect(req.get(), &jcon::JsonRpcRequest::error, this, &QIperfC::onRPC_error);
//            auto rs = rpc_tp->rpc->callNamedParams("start", QVariantMap{{"idx", result->result()}});
//            if (!rs->isSuccess()){
//                emit errorStop(-1, "start Iperf server at " + mhost +
//                               " with " +rpc_tp->tp->getServer()+ ":" + rpc_tp->tp->getPort() +
//                               " fail");
//                return;
//            }
        }
        foreach (QString mhost, map_qiperfds_client.keys()) {
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            //TODO: wait client ready/start
            qDebug() <<" Start iperf client: "<< mhost << Qt::endl;
            auto rpc_tp = map_qiperfds_client.value(mhost);
            // Add Iperf client
            auto result = rpc_tp->rpc->callNamedParams("addIperfClient",
                                            QVariantMap{{"version",rpc_tp->tp->getVersion()},
                                                        {"port",rpc_tp->tp->getPort()},
                                                        {"Host", rpc_tp->tp->getServer()},
                                                        {"iperfargs",rpc_tp->tp->getClientArgs()}});
            if (!result->isSuccess()){
                emit errorStop(-1, "addIperfClient at " + mhost + " with: " +rpc_tp->tp->getClientArgs());
                return;
            }
        }
        //TODO: start all Iperf client
        foreach (QString mhost, map_qiperfds_client.keys()) {
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            auto rpc_tp = map_qiperfds_client.value(mhost);
            auto rs = rpc_tp->rpc->callAsync("startAll");
        }
#endif


    } else {
        QMessageBox::information(this,"NOTICE", "Plase add iperf test pair first!");
    }
}

void QIperfC::onStop(){
    QString cmd="";
    foreach (auto key, m_wsc.keys()){
            qDebug() << "m_wsc: " << key;
            if (m_wsc[key]){
                cmd = QString(CMD_IPERF_STOP)+":" + key;
                qDebug() << m_wsc[key] << " m_wsc send cmd: " << cmd;
                m_wsc[key]->sendText(cmd);
            }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    foreach (auto key, m_wss.keys()){
            qDebug() << "m_wss: " << key;
            if (m_wss[key]){
                cmd = QString(CMD_IPERF_STOP)+":" + key;
                qDebug() << m_wss[key] <<  "m_wss send cmd: " << cmd;
                m_wss[key]->sendText(cmd);
            }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    updateRunStatus(false);
    //TODO: stop the running test!!
    QString endtime = getNowString();
    QDateTime enddatetime = QDateTime::fromString(endtime,DATETIME_NOW_FORMAT);
    emit updateStatus("Finish at  "+ endtime +" (Runtime: "+QString::number(m_TestStartTime.secsTo(enddatetime))+" sec)");
    //stop test
//    m_tpmgr->stop();
    // check all client endpoint stop
    // force stop all client endpoint
}

void QIperfC::onClear(){
    //clear all test date, config setting remain unchanged
    if (m_tpmgr->rootChildCount()>0) {
        m_tpmgr->clear();
        ui->tv_throughput->collapseAll();
    }
    m_tpplot->clear();
    m_TestStartTime = QDateTime();
    emit updateStatus("");
    emit updateStarttime("");
    ui->actionShowLog->setEnabled(false);
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
        TP *tp;
        QString s="";
        foreach(auto idx, idxs){
            tp = m_tpmgr->getItem(idx);
            s = s + "\n" + tp->getJsonData();
        }
        m_clipboard->setText(s);
    }
}

void QIperfC::onPaste()
{
    if (ui->tv_throughput->hasFocus()){
        QString clip = m_clipboard->text();
        m_tpmgr->onPaste(clip);
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
    emit updateStarttime("");
    emit updateStatus(msg);
    updateRunStatus(false);
//    onStop();
}

void QIperfC::on_notice(QString send_addr, QString msg)
{
    //receive qiperfd notices
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
    // check validity of the document
    if(!doc.isNull())
    {
        QJsonObject obj = doc.object();
        int act = obj["ACT"].toInt();
        switch (act){
            case EndPointAct::Add:
                if (m_endpointmgr->add(send_addr, msg)){
                    if (dlgiperf){
//                        qInfo() << "on_notice:EndPointAct:Add: " << send_addr << " msg: " << msg;
//                        if (dlgiperf->add(send_addr)){
                        if (dlgiperf->add(send_addr, msg)){
                            dlgiperf->updateUI();
                        }
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
        qDebug() << "TODO on_notice invalid message: from(" << send_addr << ") " << msg << Qt::endl;
    }
}

void QIperfC::onQuit()
{
    qInfo() << "onQuit" << Qt::endl;
    // TODO: do any thing before quit!
    qApp->quit();
}
#if (TEST_JSONRPC==1)
int QIperfC::createRPC_Server(TP tp, QString host, int rpc_port)
{
    jcon::JsonRpcWebSocketClient *rpcclient = new jcon::JsonRpcWebSocketClient();
    if (rpcclient->connectToServer(host, rpc_port)){
        RpcTp *rpc_tp = new RpcTp() ;// = nullptr;
        rpc_tp->setRPC(rpcclient);
        rpc_tp->setTP(tp);
        map_qiperfds_server[host] = rpc_tp;  //qiperfd of iperf server
        QObject::connect(rpcclient, &jcon::JsonRpcClient::notificationReceived,
                    this, &QIperfC::notificationReceived);
        return 0;
    } else {
        qDebug() << "createRPC_Server connect to " << host << " : " << rpc_port << " Fail" << Qt::endl;
        return -1;
    }
}
int QIperfC::createRPC_Client(TP tp, QString host, int rpc_port)
{
    jcon::JsonRpcWebSocketClient *rpcclient = new jcon::JsonRpcWebSocketClient();
    if (rpcclient->connectToServer(host, rpc_port)){
        RpcTp *rpc_tp= new RpcTp() ;
        rpc_tp->setRPC(rpcclient);
        rpc_tp->setTP(tp);
        map_qiperfds_client[host] = rpc_tp;  //qiperfd of iperf client
        return 0;
    } else {
        qDebug() << "connect to " << host << " : " << rpc_port << " Fail" << Qt::endl;
        return -1;
    }
}
#endif
void QIperfC::notificationReceived(const QString key, const QVariant value)
{
    qDebug() << "RPC Received notification:"
                     << "Key:" << key
                     << "Value:" << value << Qt::endl;
}

void QIperfC::onTest()
{
    m_dlgtest->show();
}

void QIperfC::closeEvent(QCloseEvent *event)
{
    //TODO: check config edit.
    Q_UNUSED(event);

    saveSettings();
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

void QIperfC::initCustomPlote()
{
    m_tpplot=new TPPlot(ui->widget_console);
    m_tpplot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tpplot, &TPPlot::customContextMenuRequested, this, &QIperfC::onPlotContextMenuRequest);
    ui->hl_console->addWidget(m_tpplot);
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
    m_settings->sync(); // forces to write the settings to storage
    m_settings->endGroup();
    m_settings->beginGroup("Iperf");
//    m_settings->value("WaitServerReady", 10).toInt();
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
    m_settings->endGroup();
    m_settings->beginGroup("Iperf");
    m_WaitServerReady =m_settings->value("WaitServerReady", 10).toInt();
//    m_frm_option->setWaitServerReady();
    m_settings->endGroup();
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
        qDebug() << "onIperfStoped:" << refrow << " err_no:" << err_no << " : " << err;
    }
//    m_tpmgr.setComment();

}

void QIperfC::onIperfTPdata(QString refrow, QString sInterval, QString datas)
{
    //receive iperf throughput data
    QJsonDocument doc=QJsonDocument::fromJson(datas.toUtf8());
    QJsonArray jArr = doc.array();//.object();
    foreach (auto jObj, jArr){
        bool avg=false;
        QString dir=nullptr;
        if (!jObj["dir"].isUndefined()){
            dir=jObj["dir"].toString();
        }
        if (!jObj["AVG"].isUndefined()){
            avg=jObj["AVG"].toBool();
        }
        // iperf sInterval = 0.00-1.00 format
        if (sInterval.contains("-")){
            sInterval = sInterval.right(sInterval.indexOf("-"));
        }
        //TODO treeview data
        m_tpmgr->addTPdata(refrow, sInterval, jObj["idx"].toString(),
                jObj["value"].toString(), jObj["unit"].toString(), dir);
        // chart data
        if (!avg){
            m_tpplot->onIperfTPdata(sInterval, refrow + "_" + jObj["idx"].toString(), jObj["value"].toString());
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

void QIperfC::onDisconnected(QString serverip)
{
    qDebug() << "onDisconnected: " << serverip;
    if (m_wss.contains(serverip)){
        m_wss.remove(serverip);
    }
    if (m_wsc.contains(serverip)){
        m_wsc.remove(serverip);
    }
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

void QIperfC::initActions()
{
    // init actions
    // file
    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(onNew()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(onOpen()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(onSave()));
    // edit
    connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(onCopy()));
    connect(ui->actionPaste, SIGNAL(triggered()), this, SLOT(onPaste()));

//    connect(ui->actionAdd, SIGNAL(triggered()), this, SLOT(onPairAdd()));
    connect(ui->actionAddIperf, SIGNAL(triggered()), this, SLOT(onAddIperf()));
    connect(ui->actionEdit, SIGNAL(triggered()), this, SLOT(onPairEdit()));
    connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(onPairDelete()));
    connect(ui->actionSwap, SIGNAL(triggered()), this, SLOT(onPairSwap()));

    // run
    connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(onStart()));
    connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(onStop()));
    connect(ui->actionClear, SIGNAL(triggered()), this, SLOT(onClear()));
    connect(ui->actionShowLog, SIGNAL(triggered()), this, SLOT(onShowLog()));

    //option
    connect(ui->actionConfig, SIGNAL(triggered()), this, SLOT(onConfig()));

    //help
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
    //test
    connect(ui->actionTest, SIGNAL(triggered()), this, SLOT(onTest()));

}

void QIperfC::initToolbar()
{
    QMenu *menuAdd = new QMenu(this);
    menuAdd->addAction(ui->actionAddIperf);

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
    qDebug() << "getDataType: " << tp->getDataType();
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
