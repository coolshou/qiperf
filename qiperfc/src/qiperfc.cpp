#include "qiperfc.h"
#include "qiperfc.h"
#include "ui_qiperfc.h"
#include "../src/comm.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
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
    ui->tv_throughput->setColumnWidth(TP::cols::id, 35);
    ui->tv_throughput->setColumnWidth(TP::cols::server, 200);
    ui->tv_throughput->setColumnWidth(TP::cols::client, 200);
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
    //TODO: on_pair_add
    dlgiperf->updateUI();
    int rc = dlgiperf->exec();//>show();
    if (rc == QDialog::Accepted){
//        qDebug() << "TODO: on_pairAdd Accepted" << Qt::endl;
        QString rs= dlgiperf->getJsonCfg();
//        qDebug() << "iperf json config: " << rs << Qt::endl;
        m_tpmgr->add(rs);
    }
}

void QIperfC::on_pairEdit()
{
    // TODO: edit
}

void QIperfC::on_pairDelete()
{
    QModelIndex cur = ui->tv_throughput->selectionModel()->currentIndex();
    QAbstractItemModel *model = ui->tv_throughput->model();
    qDebug() << "on_pairDelete: " << cur << Qt::endl;
    if (!model->removeRow(cur.row(), cur.parent())){
        QMessageBox::information(this, "ERROR", "Can not remove test pair: " + cur.data().toString());
    }
}

void QIperfC::onStart()
{
    if (m_tpmgr->rootChildCount()>0) {
        updateRunStatus(true);
        //start test
    //    m_tpmgr->start();
//        qDebug() << "ChildCount: " << m_tpmgr->rootChildCount() << Qt::endl;
        QList<TP *> tps = m_tpmgr->getChilds();
        TP *tp;
        foreach (tp, tps) {
            qDebug() << "Server:" << tp->getServer() << " Client:" << tp->getClient() << Qt::endl;
            qDebug() << "Mgr Server:" << tp->getMgrServer() << "Mgr Client:" << tp->getMgrClient() << Qt::endl;
            //TODO: check client ping server first

            //TODO: control all server endpoint init iperf server
            if (createRPC_Server( tp->getMgrServer())==-1){
                qDebug() << "createRPC_Server fail: " << tp->getMgrServer() << Qt::endl;
            }
            //TODO: control all client endpoint init iperf -c
            if (createRPC_Client( tp->getMgrClient())==-1){
                qDebug() << "createRPC_Client fail: " << tp->getMgrClient() << Qt::endl;
            }

        }

        foreach (QString h, map_qiperfds_server.keys()) {
            //TODO: wait server ready/start
            qDebug() <<" Start iperf server: "<< h << Qt::endl;
            auto rpc = map_qiperfds_server.value(h);
            // Add Iperf server
            auto result = rpc->call("getOS");
            if (result->isSuccess()) {
                qDebug() <<" getOS:" << result->result() << Qt::endl;
            } else {
                qDebug() <<" ERROR: " << result->toString();
            }
//            rpc.start();
        }


        foreach (QString h, map_qiperfds_client.keys()) {
            //TODO: wait client ready/start
            qDebug() <<" Start iperf client: "<< h << Qt::endl;
            // Add Iperf client
//            map_qiperfds_client.value(h);
        }
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
                qDebug() << "TODO EndPoint Update: (" << send_addr << ") " << msg << Qt::endl;
                break;
            case EndPointAct::Del:
                qDebug() << "TODO EndPoint Del: (" << send_addr << ") " << msg << Qt::endl;
                break;
            case EndPointAct::Disable:
                qDebug() << "TODO EndPoint Disable: (" << send_addr << ") " << msg << Qt::endl;
                break;
        }
    } else {
        qDebug() << "TODO on_notice invalid message: (" << send_addr << ") " << msg << Qt::endl;
    }
}

void QIperfC::onQuit()
{
    qInfo() << "onQuit" << Qt::endl;
    // TODO: do any thing before quit!
    qApp->quit();
}

int QIperfC::createRPC_Server(QString host, int port)
{
    jcon::JsonRpcWebSocketClient *rpcclient = new jcon::JsonRpcWebSocketClient();
    if (rpcclient->connectToServer(host, port)){
        map_qiperfds_server[host] = rpcclient;  //qiperfd of iperf server
        QObject::connect(rpcclient, &jcon::JsonRpcClient::notificationReceived,
                    this, &QIperfC::notificationReceived);
//        auto req = rpc_client->callAsync("getOS");
//        req->connect(req.get(), &jcon::JsonRpcRequest::result,
//                     [](const QVariant& result) {
//                         qDebug() << "result of RPC call:" << result << Qt::endl;
//                         qApp->exit();
//                     });
//        req->connect(req.get(), &jcon::JsonRpcRequest::error,
//                     [](int code, const QString& message, const QVariant& data) {
//                         qDebug() << "RPC error: " << message << " (" << code << ")" << data << Qt::endl;
//                         qApp->exit();
//                     });
        return 0;
    } else {
        qDebug() << "createRPC_Server connect to " << host << " : " << port << " Fail" << Qt::endl;
        return -1;
    }
}
int QIperfC::createRPC_Client(QString host, int port)
{
    jcon::JsonRpcWebSocketClient *rpcclient = new jcon::JsonRpcWebSocketClient();
    if (rpcclient->connectToServer(host, port)){
        map_qiperfds_client[host] = rpcclient;  //qiperfd of iperf client
//        auto req = rpc_client->callAsync("getOS");
//        req->connect(req.get(), &jcon::JsonRpcRequest::result,
//                     [](const QVariant& result) {
//                         qDebug() << "result of RPC call:" << result << Qt::endl;
//                         qApp->exit();
//                     });
//        req->connect(req.get(), &jcon::JsonRpcRequest::error,
//                     [](int code, const QString& message, const QVariant& data) {
//                         qDebug() << "RPC error: " << message << " (" << code << ")" << data << Qt::endl;
//                         qApp->exit();
//                     });
        return 0;
    } else {
        qDebug() << "connect to " << host << " : " << port << " Fail" << Qt::endl;
        return -1;
    }
}

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
  double xScale = (rand()/(double)RAND_MAX + 0.5)*2;
  double yScale = (rand()/(double)RAND_MAX + 0.5)*2;
  double xOffset = (rand()/(double)RAND_MAX - 0.5)*4;
  double yOffset = (rand()/(double)RAND_MAX - 0.5)*10;
  double r1 = (rand()/(double)RAND_MAX - 0.5)*2;
  double r2 = (rand()/(double)RAND_MAX - 0.5)*2;
  double r3 = (rand()/(double)RAND_MAX - 0.5)*2;
  double r4 = (rand()/(double)RAND_MAX - 0.5)*2;
  QVector<double> x(n), y(n);
  for (int i=0; i<n; i++)
  {
    x[i] = (i/(double)n-0.5)*10.0*xScale + xOffset;
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
    m_endpoint_label->setFrameStyle(QFrame::Box | QFrame::Sunken);
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
    } else {
        bStart=true;
    }
    updateRunStatus(bStart);
}


