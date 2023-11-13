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
#include "tpdirdelegate.h"
#include "endpointact.h"
#include "tp.h"
#include <QDebug>



QIperfC::QIperfC(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //UI actions
    init_actions();
    //dataTimer = QTimer();
    initCustomPlote();
    //
    m_tpmgr = new TPMgr(this);
    connect(m_tpmgr, &TPMgr::rowsInserted, this, &QIperfC::onTPDataUpdate);
    connect(m_tpmgr, &TPMgr::rowsRemoved, this, &QIperfC::onTPDataUpdate);
    ui->tv_throughput->setModel(m_tpmgr);
    ui->tv_throughput->setColumnWidth(TP::cols::id, 30);
    ui->tv_throughput->setColumnWidth(TP::cols::server, 180);
    ui->tv_throughput->setColumnWidth(TP::cols::dir, 80);
    ui->tv_throughput->setColumnWidth(TP::cols::client, 180);
    //following will cause problem!!
    tpdrdelegate = new TPDirDelegate(this);
    ui->tv_throughput->setItemDelegateForColumn(TP::cols::dir, tpdrdelegate);
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
    initStatusbar();
}

QIperfC::~QIperfC()
{
    delete ui;
}

void QIperfC::onNewMessage(const QString msg)
{
    ui->textEdit->append(msg);
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
    QModelIndex cur = ui->tv_throughput->selectionModel()->currentIndex();
    qDebug() << "on_pairEdit: " << cur;
}

void QIperfC::on_pairDelete()
{
    QModelIndex cur = ui->tv_throughput->selectionModel()->currentIndex();
//    qDebug() << "on_pairDelete: " << cur << Qt::endl;
    if (!m_tpmgr->removeRow(cur.row(), cur.parent())){
        QMessageBox::information(this, "ERROR", "Can not remove test pair: " + cur.data().toString());
    }
}

void QIperfC::onStart()
{
    m_TestStartTime = QDateTime::currentDateTime();
    //if (m_tpmgr->children().count()>0) {
    if (m_tpmgr->rootChildCount()>0) {
        updateRunStatus(true);
        //start test
    //    m_tpmgr->start();
//        qDebug() << "ChildCount: " << m_tpmgr->rootChildCount() << Qt::endl;
        QList<TP *> tps = m_tpmgr->getChilds();
//        TP *tp;
        QString s;
        foreach (TP *tp, tps) {
            //RPC to control all server endpoint (iperf server)
            if (!m_wss.contains(tp->getMgrServer())) {
                s = QStringLiteral("ws://%1:%2").arg(tp->getMgrServer()).arg(QIPERFD_WSPORT);
//                s = QStringLiteral("wss://%1:%2").arg(tp->getMgrServer()).arg(QIPERFD_WSPORT);  //ssl
                m_wss[tp->getMgrServer()]=new WSClient(QUrl(s));
                //tell server add iperf server
                m_wss[tp->getMgrServer()]->sendText(tp->getServerArgs());
            }
            //RPC to control all client endpoint (iperf client)
            if (!m_wsc.contains(tp->getMgrClient())) {
                s = QStringLiteral("ws://%1:%2").arg(tp->getMgrClient()).arg(QIPERFD_WSPORT);
//                s = QStringLiteral("wss://%1:%2").arg(tp->getMgrClient()).arg(QIPERFD_WSPORT); //ssl
                m_wsc[tp->getMgrClient()]=new WSClient(QUrl(s));
                //tell client add iperf client
                m_wsc[tp->getMgrServer()]->sendText(tp->getClientArgs());
            }
        }
        qint64 rs=0;
        QString key;
        //Start server
        foreach (key, m_wss.keys()){
            rs = m_wss[key]->sendText("Start");
            qDebug() << "rs: " << rs << " key:" << key;
        }
        //Start client
        foreach (key, m_wsc.keys()){
            rs = m_wsc[key]->sendText("Start");
            qDebug() << "rs: " << rs << " key:" << key;
        }

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
                                            QVariantMap{{"version",rpc_tp->tp->getVersion()},
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
        //TODO: wait all test done!!

        //TODO: check all test down!!

    } else {
        QMessageBox::information(this,"NOTICE", "Plase add iperf test pair first!");
    }
}

void QIperfC::onStop()
{
    updateRunStatus(false);
    //stop test
//    m_tpmgr->stop();
    // check all client endpoint stop
    // force stop all client endpoint
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
void QIperfC::updateRunStatus(bool bStart)
{
    ui->actionStart->setEnabled(!bStart);
    ui->actionStop->setEnabled(bStart);
}

void QIperfC::initCustomPlote()
{
    //init qcustomplot
    ui->cplot_tp->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                  QCP::iSelectLegend | QCP::iSelectPlottables);
    ui->cplot_tp->axisRect()->setupFullAxesBox();
    //x Axis
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->cplot_tp->xAxis->setTicker(timeTicker);

    //set axis Label
    ui->cplot_tp->xAxis->setLabel("Time(Sec)");
    ui->cplot_tp->yAxis->setLabel("Mbps");
    //set axis range
    // TODO: update range by throughput/time
    ui->cplot_tp->xAxis->setRange(0, 120);
    ui->cplot_tp->yAxis->setRange(0, 1000);
//    ui->cplot_tp->replot();
    // legend
    ui->cplot_tp->legend->setVisible(true);
    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
    // position the legend outside of the graph
    ui->cplot_tp->plotLayout()->addElement(0, 1, subLayout);
    ui->cplot_tp->plotLayout()->setColumnStretchFactor(1, 0.001); // col 1
    ui->cplot_tp->plotLayout()->setRowStretchFactor(0, 1); // row 0

    //    subLayout->addElement(0, 0, new QCPLayoutElement); // row 0
    subLayout->addElement(0, 0, ui->cplot_tp->legend); // row 0
    subLayout->addElement(1, 0, new QCPLayoutElement); // row 1
    subLayout->setRowStretchFactor(1, 0.001);
//    ui->cplot_tp->plotLayout()->setRowStretchFactor(2, 0.001);

    QFont legendFont = font();
    legendFont.setPointSize(10);
    ui->cplot_tp->legend->setFont(legendFont);
    ui->cplot_tp->legend->setSelectedFont(legendFont);
    ui->cplot_tp->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->cplot_tp->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->cplot_tp->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->cplot_tp->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->cplot_tp->yAxis2, SLOT(setRange(QCPRange)));

#if TEST_PLOT_DATA==1
    if (1) {
        QPen graphPen;
        ui->cplot_tp->addGraph();
        ui->cplot_tp->graph(0)->setName("TP1");
        graphPen = newColorPen(rand()%245+10, rand()%245+10, rand()%245+10, 1);
        ui->cplot_tp->graph(0)->setPen(graphPen);

        ui->cplot_tp->addGraph();
        ui->cplot_tp->graph(1)->setName("TP2");
        graphPen = newColorPen(rand()%245+10, rand()%245+10, rand()%245+10, 1);
        ui->cplot_tp->graph(1)->setPen(graphPen);
        // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
        connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
        dataTimer.start(1000); // Interval 0 means to refresh as fast as possible => 15% CPU (i5-8500 CPU @ 3.00GHz)
        // Interval 1000 => 1 s  => 0.13% CPU (i5-8500 CPU @ 3.00GHz)
    }
    if (0){
        addRandomGraph();
        addRandomGraph();
    }
    ui->cplot_tp->rescaleAxes();
#endif

}
void QIperfC::addRandomGraph()
{ // Test QCustomPlot
  int n = 50; // number of points in graph
    double xScale = (rand()/static_cast<double>(RAND_MAX) + 0.5)*2;
  double yScale = (rand()/static_cast<double>(RAND_MAX) + 0.5)*2;
    double xOffset = (rand()/static_cast<double>(RAND_MAX) - 0.5)*4;
  double yOffset = (rand()/static_cast<double>(RAND_MAX) - 0.5)*10;
    double r1 = (rand()/static_cast<double>(RAND_MAX) - 0.5)*2;
  double r2 = (rand()/static_cast<double>(RAND_MAX) - 0.5)*2;
    double r3 = (rand()/static_cast<double>(RAND_MAX) - 0.5)*2;
  double r4 = (rand()/static_cast<double>(RAND_MAX) - 0.5)*2;
  QVector<double> x(n), y(n);
  for (int i=0; i<n; i++)
  {
        x[i] = (i/static_cast<double>(n)-0.5)*10.0*xScale + xOffset;
    y[i] = (qSin(x[i]*r1*5)*qSin(qCos(x[i]*r2)*r4*3)+r3*qCos(qSin(x[i])*r4*2))*yScale + yOffset;
  }

  ui->cplot_tp->addGraph();
  ui->cplot_tp->graph()->setName(QString("New graph %1").arg(ui->cplot_tp->graphCount()-1));
  ui->cplot_tp->graph()->setData(x, y);
  ui->cplot_tp->graph()->setLineStyle(QCPGraph::lsLine);
//  ui->cplot_tp->graph()->setLineStyle((QCPGraph::LineStyle)(rand()%5+1));
//  if (rand()%100 > 50)
//    ui->cplot_tp->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(rand()%14+1)));
  QPen graphPen;
//  graphPen.setColor(QColor(rand()%245+10, rand()%245+10, rand()%245+10));
//  graphPen.setWidthF(rand()/(double)RAND_MAX*2+1);
  graphPen = newColorPen(rand()%245+10, rand()%245+10, rand()%245+10, 1);
  ui->cplot_tp->graph()->setPen(graphPen);
  ui->cplot_tp->replot();
}

QPen QIperfC::newColorPen(int r, int g, int b, int width)
{
    QPen graphPen;
    graphPen.setColor(QColor(r, g, b));
    graphPen.setWidthF(width);
    return graphPen;
}
void QIperfC::realtimeDataSlot(QPrivateSignal sig)
{ //test live data
    Q_UNUSED(sig)
    static double time = 0;
    time += 1;
    double value = QRandomGenerator::global()->generate() % 1000; // Simulated data value
    double value2 = QRandomGenerator::global()->generate() % 1000;
    // Add the data point to the graph
    ui->cplot_tp->graph(0)->addData(time, value);
    ui->cplot_tp->graph(1)->addData(time, value2);

    ui->cplot_tp->xAxis->setRange(time, 120, Qt::AlignRight);
    ui->cplot_tp->replot();
}

void QIperfC::onRPC_result(const QVariant &result)
{
    qDebug() << "onRPC_result: " << result << Qt::endl;
}

void QIperfC::onRPC_error(int code, const QString &message)
{
    qDebug() << "onRPC_error: (" << code << ")" << message << Qt::endl;
}

void QIperfC::init_actions()
{
    // init actions
    connect(ui->actionAdd, SIGNAL(triggered()), this, SLOT(on_pairAdd()));
    connect(ui->actionEdit, SIGNAL(triggered()), this, SLOT(on_pairEdit()));
    connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(on_pairDelete()));

    //start/stop
    connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(onStart()));
    connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(onStop()));
}

void QIperfC::initStatusbar()
{
    // statusbar of endpints
    m_endpoint_label = new QLabel(this);
//TODO: double click
    m_endpoint_label->setText("endpoints:0");
    m_endpoint_label->setFrameStyle(static_cast<int>(QFrame::Box) | static_cast<int>(QFrame::Sunken));
//    m_endpoint_label->setTextFormat(Qt::RichText);
//    m_endpoint_label->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(m_endpoint_label);

    connect(this , SIGNAL(updateEndpointNum(int)), this, SLOT(on_updateEndpointNum(int)));
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
