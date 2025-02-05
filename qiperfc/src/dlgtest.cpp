#include "dlgtest.h"
#include "ui_dlgtest.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "comm.h"

DlgTest::DlgTest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgTest)
{
    ui->setupUi(this);
    connect(ui->pb_test, &QPushButton::clicked, this, &DlgTest::onTest);
}

DlgTest::~DlgTest()
{
    delete ui;
}

void DlgTest::append(QString msg)
{
    ui->textEdit->append(msg);
}

void DlgTest::onTest()
{
    //    QString s="[{\"idx\":\"7\",\"unit\":\"Mbits/sec\",\"value\":\"22.3\"},{\"idx\":\"10\",\"unit\":\"Mbits/sec\",\"value\":\"22.2\"},{\"idx\":\"12\",\"unit\":\"Mbits/sec\",\"value\":\"22.0\"}]";

    //    onIperfTPdata("0", "0.0-1.0", s);
    //    ui->pb_test->setEnabled(true);

}

void DlgTest::on_pb_add_server_clicked()
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

    iperfObj.insert("cmd_args", cmd_args);
    mainObj.insert("Action", CMD_IPERF_ADD);
    mainObj.insert("iperf", iperfObj);

    QJsonDocument doc(mainObj);
    QString strJson(doc.toJson(QJsonDocument::Compact));

//    pclient->send_MessageToServer(strJson);
}

void DlgTest::on_pb_stop_clicked()
{
    QJsonObject mainObj;
    mainObj.insert("Action", CMD_IPERF_STOP);
    QJsonDocument doc(mainObj);
    QString strJson(doc.toJson(QJsonDocument::Compact));
//    pclient->send_MessageToServer(strJson);

}
void DlgTest::on_pb_status_clicked()
{
//    pclient->send_MessageToServer(CMD_STATUS);
}


void DlgTest::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
