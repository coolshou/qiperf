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
#define TEST_JSONRPC 0

QIperfC::QIperfC(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //UI actions
    init_actions();

    m_tpmgr = new TPMgr();
    ui->tv_throughput->setModel(m_tpmgr);
    ui->tv_throughput->setColumnWidth(TP::cols::id, 35);
    ui->tv_throughput->setColumnWidth(TP::cols::server, 200);
    ui->tv_throughput->setColumnWidth(TP::cols::client, 200);
    //following will cause problem!!
//    TPDirDelegate tpdrdelegate;
//    ui->tv_throughput->setItemDelegateForColumn(TP::cols::dir, &tpdrdelegate);
    QItemSelectionModel *ism = ui->tv_throughput->selectionModel();
    connect(ism, &QItemSelectionModel::selectionChanged, this, &QIperfC::onTPselectionChanged);
//    ui->tv_throughput->header()->setVisible(true);

    m_endpointmgr = new EndPointMgr();
    ui->tv_qiperfd->setModel(m_endpointmgr);
    ui->tv_qiperfd->setColumnWidth(0, 130);
    ui->tv_qiperfd->setColumnWidth(1, 130);

    dlgiperf = new DlgIperf();
    formEndpoits = new FormEndPoints();
    //
    m_receiver = new UdpReceiver(QIPERFD_BPORT,this);
    connect(m_receiver, &UdpReceiver::notice, this, &QIperfC::on_notice);

    // control local qiperfd?
    pclient = new PipeClient(QIPERFD_NAME);
    connect(pclient, SIGNAL(newMessage(QString)), this, SLOT(onNewMessage(QString)));
    pclient->SetAppHandle(qApp);

    // control remote qiperfd?
    if (TEST_JSONRPC){
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
    }

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
        qDebug() << "ChildCount: " << m_tpmgr->rootChildCount() << Qt::endl;
        QList<TP *> tps = m_tpmgr->getChilds();
        TP *tp;
        foreach (tp, tps) {
            qDebug() << "MServer:" << tp->getManagerServer() << " MClient:" << tp->getManagerClient() << Qt::endl;
        }
        //TODO: control all server endpoint init iperf server
        //TODO: wait server ready
        //TODO: control all client endpoint init iperf -c
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

void QIperfC::updateRunStatus(bool bStart)
{
    ui->actionStart->setEnabled(!bStart);
    ui->actionStop->setEnabled(bStart);
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

