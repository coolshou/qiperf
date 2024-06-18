#include "qiperfc.h"
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
#include <QMessageBox>
#include <QThread>
#include <QCoreApplication>
#include <QEventLoop>
#include <QTreeView>

#include "tpdirdelegate.h"
#include "endpointact.h"
#include "tp.h"
#include "versions.h"

#include <QDebug>



QIperfC::QIperfC(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    QString settingfilepath =  QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir d{settingfilepath};
    if (!d.exists()){
        if(!d.mkpath(settingfilepath)){
            qDebug() << "ERROR: mkdir " + settingfilepath + " Fail";
        }
    }
    QString settingfilename = settingfilepath + "/" + QIPERFC_NAME + ".ini";
    m_settings=new QSettings(settingfilename, QSettings::IniFormat);
    ui->setupUi(this);
    initStatusbar();
    loadSettings();
    m_qipconfig = new QIPConfig();
    //UI actions
    init_actions();
    //dataTimer = QTimer();
    initCustomPlote();
    connect(this, &QIperfC::errorStop, this, &QIperfC::onErrorStop);

    iTimeout = 10*100;
    //
    m_tpmgr = new TPMgr(this);
    connect(m_tpmgr, &TPMgr::rowsInserted, this, &QIperfC::onTPDataUpdate);
    connect(m_tpmgr, &TPMgr::rowsRemoved, this, &QIperfC::onTPDataUpdate);


    ui->tv_throughput->setModel(m_tpmgr);
    ui->tv_throughput->setColumnWidth(TP::cols::id, 30);
    ui->tv_throughput->setColumnWidth(TP::cols::server, 180);
    ui->tv_throughput->setColumnWidth(TP::cols::dir, 80);
    ui->tv_throughput->setColumnWidth(TP::cols::client, 180);
    connect(ui->tv_throughput, &QTreeView::doubleClicked, this, &QIperfC::onItemClicked);

    //following will cause problem!! slow update image?
//    tpdrdelegate = new TPDirDelegate(this);
//    ui->tv_throughput->setItemDelegateForColumn(TP::cols::dir, tpdrdelegate);

    QItemSelectionModel *ism = ui->tv_throughput->selectionModel();
    connect(ism, &QItemSelectionModel::selectionChanged, this, &QIperfC::onTPselectionChanged);
//    ui->tv_throughput->header()->setVisible(true);

    m_endpointmgr = new EndPointMgr(this);
    ui->tv_qiperfd->setModel(m_endpointmgr);
    ui->tv_qiperfd->setColumnWidth(0, 130);
    ui->tv_qiperfd->setColumnWidth(1, 130);

    dlgiperf = new DlgIperf(this);
    formEndpoits = new FormEndPoints();
    //
    m_receiver = new UdpReceiver(QIPERFD_BPORT,this);
    connect(m_receiver, &UdpReceiver::notice, this, &QIperfC::on_notice);

    // control local qiperfd?
    pclient = new PipeClient(QIPERFD_NAME);
    connect(pclient, SIGNAL(newMessage(QString)), this, SLOT(onNewMessage(QString)));
    pclient->SetAppHandle(qApp);

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
    qDebug() << "~QIperfC";

    delete ui;
}

bool QIperfC::load(QString filename)
{
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
              on_Save();
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
        QByteArray b = m_qipconfig->getTPCfg();
        m_tpmgr->loaddata(b);
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
        m_qipconfig->setTPCfg(b);

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
    ui->textEdit->append(msg);
}

void QIperfC::on_New()
{
    //TODO: check tp config exist?
    on_Clear();
}

void QIperfC::on_Open()
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

void QIperfC::on_Save()
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
    qDebug() << "save file: " << fileName << Qt::endl;
    save(fileName);

}

void QIperfC::on_Clear()
{
    if (m_tpmgr->rootChildCount()>0) {
        m_tpmgr->reset();
    }else{
        qDebug() << "on_Clear No child";
    }
    // TODO: clear chart!!
}

void QIperfC::on_pairAdd()
{
    // on_pair_add
    dlgiperf->updateUI();
    int rc = dlgiperf->exec();// show dlgiperf
    if (rc == QDialog::Accepted){
        QString rs= dlgiperf->getJsonCfg();
        m_tpmgr->add(rs);
    }
}

void QIperfC::on_pairEdit()
{
    // TODO: edit
    QModelIndex idx = ui->tv_throughput->selectionModel()->currentIndex();
//    qDebug() << "on_pairEdit: " << cur;
    onItemClicked(idx);
}

void QIperfC::on_pairDelete()
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
    ui->tv_throughput->update();
}

void QIperfC::onStart()
{
    resetError();
    m_TestStartTime = QDateTime::currentDateTime();
    m_tpplot->setStartTime(m_TestStartTime);
    QString startTime = m_TestStartTime.toString(DATETIME_NOW_FORMAT);
    emit updateStarttime(startTime);
    //if (m_tpmgr->children().count()>0) {
    if (m_tpmgr->rootChildCount()>0) {
        updateRunStatus(true);
        //start test
        QList<TP *> tps = m_tpmgr->getChilds();
        QString s;
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
            if (!m_wss.contains(serverIP)) {
                s = "ws://"+serverIP+":"+QString::number(QIPERFD_WSPORT);
                qDebug() << "server websocket url: " << s << Qt::endl;
                m_wss[serverIP]=new WSClient(serverIP, QUrl(s));
                connect(m_wss[serverIP], &WSClient::iperfStarted, this, &QIperfC::onIperfStarted);
                connect(m_wss[serverIP], &WSClient::disconnected, this, &QIperfC::onDisconnected);
                connect(m_wss[serverIP], &WSClient::iperfTPdata, this, &QIperfC::onIperfTPdata);
                itimeout = iTimeout;
                while (! m_wss[serverIP]->isConnected() && itimeout>0){
//                    qDebug() << "wait WSClient:" << s << " connected";
                    QThread::msleep(10);
                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                    itimeout--;
                }
                if (itimeout<=0){
                    emit errorStop(1,"Wait connect to " +s+ " timeout");
                    break;
                }
                //tell server add iperf server
                QString cmd = QString(CMD_IPERF_ADD)+":"+QString::number(refrow)+":"+tp->getServerArgs();
                rs = m_wss[serverIP]->sendText(cmd);
                if (rs<=0){
                    emit errorStop(1, "Setup server iperf config fail: "+ tp->getServerArgs());
                    break;
                }
            }
            //RPC to control all client endpoint (iperf client)
            QString clientIP = tp->getMgrClient();
            if (!m_wsc.contains(clientIP)) {
                s = "ws://"+clientIP+":"+QString::number(QIPERFD_WSPORT);
                qDebug() << "client websocket url: " << s << Qt::endl;
                m_wsc[clientIP]=new WSClient(clientIP, QUrl(s));
                connect(m_wsc[clientIP], &WSClient::iperfStarted, this, &QIperfC::onIperfStarted);
                connect(m_wsc[clientIP], &WSClient::disconnected, this, &QIperfC::onDisconnected);
                connect(m_wsc[clientIP], &WSClient::iperfTPdata, this, &QIperfC::onIperfTPdata);
                itimeout = iTimeout;
                while (! m_wsc[clientIP]->isConnected()&& itimeout>0){
//                    qDebug() << "wait WSClient:" << s << " connected";
                    QThread::msleep(10);
                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                }
                if (itimeout<=0){
                    emit errorStop(1,"Wait connect to " +s+ "timeout");
                    break;
                }
                //tell client add iperf client
                QString cmd = QString(CMD_IPERF_ADD)+":"+QString::number(refrow)+":"+tp->getClientArgs();
                rs = m_wsc[clientIP]->sendText(cmd);
                if (rs<=0){
                    emit errorStop(2, "Setup client iperf config fail: "+ tp->getClientArgs());
                    break;
                }
            }
            // TODO. set report ??
            QString di = tp->getDirection();
            if (di== QVariant::fromValue(TP::DirType::Tx).toString()){
                m_wss[serverIP]->sendText(CMD_IPERF_REG);
            }else if (di== QVariant::fromValue(TP::DirType::Rx).toString()){
                m_wsc[clientIP]->sendText(CMD_IPERF_REG);
            }else if (di== QVariant::fromValue(TP::DirType::TR).toString()){
                qDebug()<<"TODO: bidir monitor";
            }else {
                emit errorStop(3, "Wrong setting of iperf direction: "+ di);
            }
        }
        //TODO: record which should report iperf throughput value

        if(bErrorStop>0){
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
            auto rpc_tp = map_qiperfds_client.value(mhost);
            auto rs = rpc_tp->rpc->callAsync("startAll");
        }
#endif


    } else {
        QMessageBox::information(this,"NOTICE", "Plase add iperf test pair first!");
    }
}

void QIperfC::onStop()
{
    updateRunStatus(false);
    QString e = getNowString();
    QDateTime enddatetime = QDateTime::fromString(e,DATETIME_NOW_FORMAT);
    emit updateStatus("Finish at  "+ e +" (Runtime: "+enddatetime.secsTo(m_TestStartTime)+" sec)");
    //stop test
//    m_tpmgr->stop();
    // check all client endpoint stop
    // force stop all client endpoint
}

void QIperfC::onClear()
{
//    on_Clear();
    //TODO: clear  m_tpmgr throughput data
//    m_tpplot->clearGraphs();
    m_tpplot->clear();
    emit updateStatus("");
    emit updateStarttime("");
}

void QIperfC::onAbout()
{
    QMessageBox::about(this, "About", QString(QIPERFC_NAME)+"\n"
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
    onStop();
}

void QIperfC::on_notice(QString send_addr, QString msg)
{
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
    // check validity of the document
    if(!doc.isNull())
    {
        QJsonObject obj = doc.object();
        int act = obj["ACT"].toInt();
        switch (act){
            case EndPointAct::Add:
//                qInfo() << "EndPointAct::Add: " << send_addr << " msg: " << msg;
                if (m_endpointmgr->add(send_addr, msg)){
                    if (dlgiperf){
                        if (dlgiperf->add(send_addr)){
                            dlgiperf->updateUI();
                        }
                    }
                    emit updateEndpointNum(m_endpointmgr->getTotalEndpoints());
                }
                break;
            case EndPointAct::Update:
                qDebug() << "TODO EndPoint Update: from(" << send_addr << ") " << msg << Qt::endl;
                break;
            case EndPointAct::Del:
                qDebug() << "TODO EndPoint Del: from(" << send_addr << ") " << msg << Qt::endl;
                break;
            case EndPointAct::Disable:
                qDebug() << "TODO EndPoint Disable: from(" << send_addr << ") " << msg << Qt::endl;
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

void QIperfC::closeEvent(QCloseEvent *event)
{
    //TODO: check config edit.
    Q_UNUSED(event);

    saveSettings();
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
}

void QIperfC::onRPC_result(const QVariant &result)
{
    qDebug() << "onRPC_result: " << result << Qt::endl;
}

void QIperfC::onRPC_error(int code, const QString &message)
{
    qDebug() << "onRPC_error: (" << code << ")" << message << Qt::endl;
}

void QIperfC::onIperfStarted(QString ipport)
{
    qDebug() << "onIperfStarted:" << ipport;
}

void QIperfC::onIperfStoped(QString ipport)
{
    qDebug() << "onIperfStoped:" << ipport;
}

void QIperfC::onIperfTPdata(QString refrow, QString sInterval, QString data)
{
//    qDebug() << "onIperfTPdata:" << refrow << " : " << data;
    QJsonDocument doc=QJsonDocument::fromJson(data.toUtf8());
    QJsonArray jArr = doc.array();//.object();
    foreach (auto jObj, jArr){
//        qDebug() << "idx: " << jObj["idx"] << " value: " << jObj["value"]
//                 << "unit: " << jObj["unit"] << " dir: " << jObj["dir"];
        QString dir=nullptr;
        if (!jObj["dir"].isUndefined()){
            dir=jObj["dir"].toString();
        }

        Q_UNUSED(refrow)
        //Q_UNUSED(sInterval)
        //TODO treeview data
//        m_tpmgr->addTPdata(refrow, sInterval,
//                           jObj["idx"].toString(),
//                jObj["value"].toString(), jObj["unit"].toString(),
//                dir);
        //TODO chart data
        // iperf sInterval = 0.00-1.00 format
        if (sInterval.contains("-")){
            sInterval = sInterval.right(sInterval.indexOf("-"));
        }
//        qDebug()<< "sInterval: " << sInterval;
        m_tpplot->addTPData(sInterval, refrow + "_" + jObj["idx"].toString(), jObj["value"].toString());
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

void QIperfC::onDisconnected(QString serverip)
{
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

void QIperfC::init_actions()
{
    // init actions
    // file
    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(on_New()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(on_Open()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(on_Save()));
    //
    connect(ui->actionAdd, SIGNAL(triggered()), this, SLOT(on_pairAdd()));
    connect(ui->actionEdit, SIGNAL(triggered()), this, SLOT(on_pairEdit()));
    connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(on_pairDelete()));
    connect(ui->actionSwap, SIGNAL(triggered()), this, SLOT(onPairSwap()));

    //start/stop
    connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(onStart()));
    connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(onStop()));
    connect(ui->actionClear, SIGNAL(triggered()), this, SLOT(onClear()));

    //help
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
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
    m_endpoint_label = new QLabel(this);
//TODO: double click
    m_endpoint_label->setText("endpoints:0");
    m_endpoint_label->setFrameStyle(static_cast<int>(QFrame::Box) | static_cast<int>(QFrame::Sunken));
//    m_endpoint_label->setTextFormat(Qt::RichText);
//    m_endpoint_label->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(m_endpoint_label);

    connect(this , &QIperfC::updateEndpointNum, this, &QIperfC::on_updateEndpointNum);
}

void QIperfC::onUpdateStarttime(QString stime)
{
    m_start_label->setText(stime);
}

void QIperfC::onUpdateStatus(QString msg)
{
    m_status_label->setText(msg);
}


void QIperfC::on_pb_status_clicked()
{
    pclient->send_MessageToServer(CMD_STATUS);
}


void QIperfC::on_pb_add_server_clicked()
{
    /*'''
    { "Action" : CMD_IPERF_ADD,
      "iperf":
        { version:3,
          port:5201,
          cmd_args:
            ["-s"]
        }
    }
    '''*/
    QJsonObject mainObj;
    QJsonObject iperfObj;
    iperfObj.insert("version", 3);
    iperfObj.insert("port", 5201);
    QJsonArray cmd_args;
    cmd_args.push_back("-s");
//    cmd_args.push_back("--forceflush");

    iperfObj.insert("cmd_args", cmd_args);
    mainObj.insert("Action", CMD_IPERF_ADD);
    mainObj.insert("iperf", iperfObj);

    QJsonDocument doc(mainObj);
    QString strJson(doc.toJson(QJsonDocument::Compact));

    pclient->send_MessageToServer(strJson);
}


void QIperfC::on_pb_start_clicked()
{
    QJsonObject mainObj;
    mainObj.insert("Action", CMD_IPERF_START);
    QJsonDocument doc(mainObj);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    pclient->send_MessageToServer(strJson);
}


void QIperfC::on_pb_stop_clicked()
{
    QJsonObject mainObj;
    mainObj.insert("Action", CMD_IPERF_STOP);
    QJsonDocument doc(mainObj);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    pclient->send_MessageToServer(strJson);

}

void QIperfC::on_updateEndpointNum(int n)
{
//    qDebug() << "on_updateEndpointNum: " << n << Qt::endl;
    m_endpoint_label->setText("endpoints:" + QString::number(n));
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
    } else {
        ui->actionStart->setEnabled(false);
        ui->actionStop->setEnabled(false);
    }
    //    updateRunStatus(bStart);
}

void QIperfC::onItemClicked(QModelIndex idx)
{
    TP *tp = m_tpmgr->getItem(idx);
    dlgiperf->loadJsonCfg(tp->saveData());
    int rc = dlgiperf->exec();// show dlgiperf
    if (rc == QDialog::Accepted){
        QString rs= dlgiperf->getJsonCfg();
        tp->loadData(rs);
    }
}
