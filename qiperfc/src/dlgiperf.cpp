#include "dlgiperf.h"
#include "ui_dlgiperf.h"
#include "../src/comm.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QComboBox>
#include <QCheckBox>

DlgIperf::DlgIperf(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgIperf)
{
    ui->setupUi(this);
    connect(ui->cb_version, &QComboBox::currentTextChanged, this, &DlgIperf::ChangeVersion);
    connect(ui->chk_bidir, &QCheckBox::stateChanged, this, &DlgIperf::on_chk_bidir_statech);
    connect(ui->chk_reverse, &QCheckBox::stateChanged, this, &DlgIperf::on_chk_reverse_statech);

//    connect(ui, &QDialog::accepted, this, &QDialog::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DlgIperf::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // TODD: temp disable item of UDP/SCTP
    auto * model = qobject_cast<QStandardItemModel*>(ui->cb_protocal->model());
    auto * itemUTP = model->item(1);
    itemUTP->setEnabled(false);
    auto * itemSCTP = model->item(2);
    itemSCTP->setEnabled(false);
//    ui->cb_protocal->model()->item(1);
}

DlgIperf::~DlgIperf()
{
    delete ui;
}

QString DlgIperf::getJsonCfg()
{
    //return Json config of iperf pair from UI's value
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
    Q_UNUSED(jsoncfg)
}

bool DlgIperf::add(QString mgr)
{
    // add manager ip address
    if (mgrls.indexOf(mgr)<0){
        mgrls.append(mgr);
        return true;
    }
    return false;
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

//void DlgIperf::closeEvent(QCloseEvent *event)
//{



//}

void DlgIperf::updateUI()
{
    //update UI of manager ip address
    ui->cb_mserver_ip->clear();
    ui->cb_mserver_ip->addItems(mgrls);
    ui->cb_mclient_ip->clear();
    ui->cb_mclient_ip->addItems(mgrls);
}

void DlgIperf::ChangeVersion(const QString ver)
{
    int port=5201;
    if (ver.indexOf("2")==0){
        port=5001;
    }
    ui->sb_port->setValue(port);
}

void DlgIperf::onAccepted()
{
    bool close=true;
    //check require fields value
    QHostAddress addr;
    if (!addr.setAddress(ui->le_target_ip->text())){
        QMessageBox::warning(this, tr("WARNING!!"),
                             tr("Please specify iperf server ip address!!"),
                             QMessageBox::Ok);
        ui->le_target_ip->setFocus();
//        event->ignore();
//        abort();
//        close = false;
        return;
    }
    if (!addr.setAddress(ui->cb_client_bind_ip->currentText())){
        QMessageBox::warning(this, tr("WARNING!!"),
                             tr("Please specify iperf client bind ip address!!"),
                             QMessageBox::Ok);
        ui->cb_client_bind_ip->setFocus();
//        close = false;
        return;
    }
    if (close){
        accept();
    }
}

void DlgIperf::on_chk_bidir_statech(int state)
{
    if (state==Qt::Checked){
        ui->chk_reverse->setCheckState(Qt::Unchecked);
    }
}

void DlgIperf::on_chk_reverse_statech(int state)
{
    if (state==Qt::Checked){
        ui->chk_bidir->setCheckState(Qt::Unchecked);
    }
}
