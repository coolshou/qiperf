#include "qiperfc.h"
#include "ui_qiperfc.h"
#include "../src/comm.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>


#include <QDebug>
#define TEST_JSONRPC 0

QIperfC::QIperfC(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_tpchart= new TPChart(this);
    ui->l_chart->addWidget(m_tpchart);
    ui->l_chart->setSizeConstraint(QLayout::SetMaximumSize);
    init_actions();

    dlgiperf = new DlgIperf();
    //
    m_receiver = new UdpReceiver(QIPERFD_BPORT,this);
//    connect(m_receiver, SIGNAL(notice()), this, SLOT(on_notice()));
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
    connect(ui->treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(On_itemSelectionChanged()));

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
    int rc = dlgiperf->exec();//>show();
    if (rc == QDialog::Accepted){
        qDebug() << "TODO: on_pairAdd Accepted" << Qt::endl;
    }
}

void QIperfC::on_pairEdit()
{
    // TODO: edit
}

void QIperfC::on_pairDelete()
{
    // TODO: delete
}

void QIperfC::on_notice(QString msg)
{
    qDebug() << "TODO on_notice: " << msg << Qt::endl;
}

void QIperfC::init_actions()
{
    // init actions

    connect(ui->actionAdd, SIGNAL(triggered()), this, SLOT(on_pairAdd()));
    connect(ui->actionEdit, SIGNAL(triggered()), this, SLOT(on_pairEdit()));
    connect(ui->actionDelete, SIGNAL(triggered()), this, SLOT(on_pairDelete()));
}


void QIperfC::on_pushButton_clicked()
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

void QIperfC::On_itemSelectionChanged()
{
    if (ui->treeWidget->selectedItems().count()>0){
        ui->actionDelete->setEnabled(true);
        ui->actionEdit->setEnabled(true);
    } else {
        ui->actionDelete->setEnabled(false);
        ui->actionEdit->setEnabled(false);
    }
}

