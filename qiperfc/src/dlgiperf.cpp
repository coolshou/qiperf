#include "dlgiperf.h"
#include "ui_dlgiperf.h"
#include "../src/comm.h"

#include <QJsonObject>
#include <QJsonDocument>

DlgIperf::DlgIperf(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgIperf)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

}

DlgIperf::~DlgIperf()
{
    delete ui;
}

QString DlgIperf::getJsonCfg()
{
    //return Json config of iperf pair
    QJsonObject mainObj;
    mainObj.insert("Action", CMD_IPERF_ADD);

    if (ui->gb_control_server->isChecked()){
        //server
        QJsonObject serverObj;
        serverObj.insert("version", ui->cb_version->currentText());
        serverObj.insert("port", ui->sb_port->value());
        serverObj.insert("manager", ui->cb_mserver_ip->currentText());
        serverObj.insert("bind", ui->cb_server_bind_ip->currentText());
        mainObj.insert("server", serverObj);
    }
    //client
    QJsonObject clientObj;
    clientObj.insert("version", ui->cb_version->currentText());
    clientObj.insert("port", ui->sb_port->value());
    clientObj.insert("manager", ui->cb_mclient_ip->currentText());
    clientObj.insert("bind", ui->cb_client_bind_ip->currentText());
    clientObj.insert("protocal", ui->cb_protocal->currentText());
    clientObj.insert("target", ui->le_target_ip->text());
    clientObj.insert("duration", ui->sb_duration->value());
    clientObj.insert("omit", ui->sb_omit->value());
    clientObj.insert("parallel", ui->sb_parallel->value());
    clientObj.insert("bitrate", ui->sb_bitrate->value());
    clientObj.insert("unit_bitrate", ui->cb_unit_bitrate->currentText());
    clientObj.insert("windowsize", ui->sb_windowsize->value());
    clientObj.insert("unit_windowsize", ui->cb_unit_windowsize->currentText());
    clientObj.insert("buffer", ui->sb_buffer->value());
    clientObj.insert("unit_buffer", ui->cb_unit_buffer->currentText());
    clientObj.insert("dscp", ui->sb_dscp->value());
    clientObj.insert("tos", ui->sb_tos->value());
    clientObj.insert("mss", ui->sb_mss->value());
    clientObj.insert("interval", ui->sb_interval->value());
    clientObj.insert("fmtreport", ui->cb_fmtreport->currentText());

    clientObj.insert("reverse", ui->chk_reverse->isChecked());
    clientObj.insert("bidir", ui->chk_bidir->isChecked());
    mainObj.insert("client", clientObj);
    QJsonDocument doc(mainObj);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

void DlgIperf::loadJsonCfg(QString jsoncfg)
{
    //TODO
}

void DlgIperf::changeEvent(QEvent *e)
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
