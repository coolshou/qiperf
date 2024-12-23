#include "dlgiperf.h"
#include "ui_dlgiperf.h"
#include "../src/comm.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QHostAddress>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QComboBox>
#include <QCheckBox>
#include <QCoreApplication>
#include <QEventLoop>

DlgIperf::DlgIperf(TPMgr *tpmgr, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgIperf)
{
    ui->setupUi(this);
    m_tpmgr = tpmgr;
    b_ipv6 = false;
    connect(ui->cb_version, &QComboBox::currentTextChanged, this, &DlgIperf::ChangeVersion);
    connect(ui->chk_bidir, &QCheckBox::stateChanged, this, &DlgIperf::onChkBidirStatech);
    connect(ui->chk_reverse, &QCheckBox::stateChanged, this, &DlgIperf::onChkReverseStatech);

//    connect(ui, &QDialog::accepted, this, &QDialog::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DlgIperf::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

//    connect(ui->cb_mserver_ip, QOverload<int>::of(&QComboBox::currentIndexChanged),
//            this,&DlgIperf::onSelectMServer);
    connect(ui->cb_mserver_ip, &QComboBox::currentTextChanged, this,&DlgIperf::onSelectMServer);
    connect(ui->cb_mclient_ip, &QComboBox::currentTextChanged, this,&DlgIperf::onSelectMClient);

    // TODD: temp disable item of UDP/SCTP
//    auto * model = qobject_cast<QStandardItemModel*>(ui->cb_protocal->model());
//    auto * itemUTP = model->item(1);
//    itemUTP->setEnabled(false);
//    auto * itemSCTP = model->item(2);
//    itemSCTP->setEnabled(false);
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
    mainObj.insert("enabled", true);
//    if (ui->gb_control_server->isChecked())
    {
        //server
        QJsonObject serverObj;
        serverObj.insert("version", ui->cb_version->currentText());
        serverObj.insert("port", ui->sb_port->value());
        serverObj.insert("manager", ui->cb_mserver_ip->currentText());
        serverObj.insert("protocal", ui->cb_protocal->currentText());
        serverObj.insert("parallel", ui->sb_parallel->value());
        serverObj.insert("reverse", ui->chk_reverse->isChecked());
        serverObj.insert("bidir", ui->chk_bidir->isChecked());
        serverObj.insert("interval", ui->sb_interval->value());
//        serverObj.insert("ipv6", b_ipv6);
        if (ui->chk_server_bind_ip->isChecked()){
            serverObj.insert("bind", ui->cb_target_ip->currentText());
        }
        serverObj.insert("fmtreport", ui->cb_fmtreport->currentText());

        serverObj.insert("delaytime", ui->sb_delaytime->value());
        mainObj.insert("server", serverObj);

    }
    //client
    QJsonObject clientObj;
    clientObj.insert("version", ui->cb_version->currentText());
    clientObj.insert("port", ui->sb_port->value());
    clientObj.insert("manager", ui->cb_mclient_ip->currentText());
    clientObj.insert("ipv6", b_ipv6);
    if (!ui->cb_client_bind_ip->currentText().isEmpty()) {
        //TODO: check IPv4/IPv6format
        clientObj.insert("bind", ui->cb_client_bind_ip->currentText());
    }
    clientObj.insert("protocal", ui->cb_protocal->currentText());
    clientObj.insert("target", ui->cb_target_ip->currentText());
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

    clientObj.insert("delaytime", ui->sb_delaytime->value());
    mainObj.insert("client", clientObj);
    QJsonDocument doc(mainObj);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

void DlgIperf::loadJsonCfg(QString jsoncfg)
{
    QJsonParseError error;
    QJsonDocument doc=QJsonDocument::fromJson(jsoncfg.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
        QJsonObject mainObj = doc.object();
        QJsonObject serverObj = mainObj["server"].toObject();
        ui->cb_version->setCurrentText(serverObj["version"].toString());
        ui->sb_port->setValue(serverObj["port"].toInt());
        ui->cb_mserver_ip->setCurrentText(serverObj["manager"].toString());
        if (!serverObj["bind"].toString().isEmpty()){
            ui->chk_server_bind_ip->setChecked(true);
        }
        ui->cb_protocal->setCurrentText(serverObj["protocal"].toString());
        ui->sb_parallel->setValue(serverObj["parallel"].toInt());
        ui->chk_bidir->setChecked(serverObj["bidir"].toBool());
        ui->sb_interval->setValue(serverObj["interval"].toInt());

        ui->sb_delaytime->setValue(serverObj["delaytime"].toInt());

        QJsonObject clientObj = mainObj["client"].toObject();
        ui->cb_mclient_ip->setCurrentText(clientObj["manager"].toString());
        ui->cb_client_bind_ip->setCurrentText(clientObj["bind"].toString());
        ui->cb_target_ip->setCurrentText(clientObj["target"].toString());
        ui->sb_duration->setValue(clientObj["duration"].toInt());
        ui->sb_omit->setValue(clientObj["omit"].toInt());
        ui->sb_bitrate->setValue(clientObj["bitrate"].toInt());
        ui->cb_unit_bitrate->setCurrentText(clientObj["unit_bitrate"].toString());
        ui->sb_windowsize->setValue(clientObj["windowsize"].toInt());
        ui->cb_unit_windowsize->setCurrentText(clientObj["unit_windowsize"].toString());
        ui->sb_buffer->setValue(clientObj["buffer"].toInt());
        ui->cb_unit_buffer->setCurrentText(clientObj["unit_buffer"].toString());
        ui->sb_dscp->setValue(clientObj["dscp"].toInt());
        ui->sb_tos->setValue(clientObj["tos"].toInt());
        ui->sb_mss->setValue(clientObj["mss"].toInt());
        ui->cb_fmtreport->setCurrentText(clientObj["fmtreport"].toString());
        ui->chk_reverse->setChecked(clientObj["reverse"].toBool());
    }else{
        qDebug() << "Wrong format of loadJsonCfg:(" << error.errorString() << ")\n" << jsoncfg;
    }

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

bool DlgIperf::add(QString mgr, QString mdata)
{
    if (add(mgr)){
        QStringList ds;
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(mdata.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonObject obj = doc.object();
            QJsonObject data;
            QJsonArray addrs;
            QString mif = obj["Manager"].toString();
            if (obj.contains("Net")){
                QJsonObject net = obj["Net"].toObject();
                foreach(const QString& key, net.keys()) {
                    if (key != mif){
                        data = net.value(key).toObject();
                        if (data.contains("address")) {
                            addrs= data.value("address").toArray();
                            if (!addrs.empty()){
                                // foreach(auto addr, addrs){
                                for (const auto &addr: addrs){
                                    if (addr.isArray()){
                                        ds.append(addr[0].toString());
                                    }
                                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                                }
                            }
                        }
                    }
                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                }
            }
        }else{
            qDebug() << "Wrong format of add:(" << error.errorString() << ")\n" << mdata;
        }
        if (ds.length()>0){
            ds.sort(Qt::CaseInsensitive);
            m_ips.insert(mgr, ds);
            return true;
        }else{
            return false;
        }
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

bool DlgIperf::isRequireConfigMet()
{
    //check require fields value
    QHostAddress addr_target;
    bool bok = addr_target.setAddress(ui->cb_target_ip->currentText());
    if (!bok){
        QMessageBox::warning(this, tr("WARNING!!"),
                             tr("Please specify iperf server ip address!!"),
                             QMessageBox::Ok);
        ui->cb_target_ip->setFocus();
        return false;
    }
    QHostAddress addr_client;
    bok =addr_client.setAddress(ui->cb_client_bind_ip->currentText());
    if (!bok){
        QMessageBox::warning(this, tr("WARNING!!"),
                             tr("Please specify iperf client bind ip address!!"),
                             QMessageBox::Ok);
        ui->cb_client_bind_ip->setFocus();
        return false;
    }
    //tmp disable IPv6 address
    if (addr_target.protocol()==QAbstractSocket::IPv6Protocol){
        QMessageBox::warning(this, tr("WARNING!!"),
                             tr("Please specify IPv4 address for iperf server!!"),
                             QMessageBox::Ok);
        ui->cb_target_ip->setFocus();
        return false;
    }
    if (addr_client.protocol()==QAbstractSocket::IPv6Protocol){
        QMessageBox::warning(this, tr("WARNING!!"),
                             tr("Please specify IPv4 address for iperf client!!"),
                             QMessageBox::Ok);
        ui->cb_client_bind_ip->setFocus();
        return false;
    }
    if (ui->cb_mserver_ip->currentText() == ui->cb_mclient_ip->currentText()){
        QMessageBox::warning(this, tr("WARNING!!"),
                             tr("Forbid setting same address of Manager Server and Manager Client!!"),
                             QMessageBox::Ok);
        ui->cb_mclient_ip->setFocus();
        //TODO: use style to hightlight some item:  *{border: 3px solid red;}
        return false;
    }
    if(ui->cb_version->currentText()=="3"){
        if (ui->chk_bidir->isChecked()){
            if (ui->sb_parallel->value()>10){
                QMessageBox::warning(this, tr("WARNING!!"),
                                     tr("In bidirectional mode, The parallel number should not over 10 (iperf3 under window may have problem)!!"),  QMessageBox::Ok);
                ui->sb_parallel->setValue(10);
                ui->sb_parallel->setFocus();
                return false;
            }
        }else{
            if (ui->sb_parallel->value()>20){
                QMessageBox::warning(this, tr("WARNING!!"),
                                     tr("In bidirectional mode, The parallel number should not over 20 (iperf3 under window may have problem)!!"),  QMessageBox::Ok);
                ui->sb_parallel->setValue(20);
                ui->sb_parallel->setFocus();
                return false;
            }
        }
    }

    //check target and client in same protocal type
    if(addr_target.protocol()!=addr_client.protocol()){
        QMessageBox::warning(this, tr("WARNING!!"),
                             tr("Please specify same protocol type of target IP and client IP!!"),
                             QMessageBox::Ok);
        // TODO: show multi focus color on following item
        ui->cb_target_ip->setFocus();
        ui->cb_client_bind_ip->setFocus();
        return false;
    }
    // TODO: check duplicate <target ip>:<port> binding!!
    QString bindkey = ui->cb_target_ip->currentText()+"_"+ QString::number(ui->sb_port->value());
    if (m_tpmgr->isBindkeyExist(ui->cb_mserver_ip->currentText(), bindkey, m_excIdx))
    {
        QString msg = ui->cb_mserver_ip->currentText() + " already have " + bindkey+ "\n Please use other value of port";
        QMessageBox::warning(this, tr("ERROR!!"), tr(msg.toUtf8()),
                             QMessageBox::Ok);
        ui->sb_port->setFocus();
        return false;
    }

    if ((addr_client.protocol()==QAbstractSocket::IPv6Protocol)&&
            (addr_target.protocol()==QAbstractSocket::IPv6Protocol)){
        b_ipv6=true;
    }
    return true;
}

void DlgIperf::updateUI()
{
    //update UI of manager ip address
    //
    QString cur = ui->cb_mserver_ip->currentText();
    ui->cb_mserver_ip->clear();
    ui->cb_mserver_ip->addItems(mgrls);
    if (!cur.isEmpty()){
        ui->cb_mserver_ip->setCurrentIndex(ui->cb_mserver_ip->findText(cur));
    }
    cur = ui->cb_mclient_ip->currentText();
    ui->cb_mclient_ip->clear();
    ui->cb_mclient_ip->addItems(mgrls);
    if (!cur.isEmpty()){
        ui->cb_mclient_ip->setCurrentIndex(ui->cb_mclient_ip->findText(cur));
    }
}

void DlgIperf::setExcIdx(QModelIndex excIdx)
{
    m_excIdx = excIdx;
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
    if (isRequireConfigMet()){
        accept();
    }else{
        return;
    }
}

void DlgIperf::onChkBidirStatech(int state)
{
    if (state==Qt::Checked){
        ui->chk_reverse->setCheckState(Qt::Unchecked);
    }
}

void DlgIperf::onChkReverseStatech(int state)
{
    if (state==Qt::Checked){
        ui->chk_bidir->setCheckState(Qt::Unchecked);
    }
}

void DlgIperf::onSelectMServer(QString text)
{
    if (!text.isEmpty()){
        ui->cb_target_ip->clear();
        if (!m_ips.isEmpty()){
            if (m_ips.contains(text)){
                QStringList ds = m_ips.value(text);
                if (ds.length()>0){
                    ui->cb_target_ip->addItems(ds);
                }
            }
        }
    }
}

void DlgIperf::onSelectMClient(QString text)
{
    if (!text.isEmpty()){
        ui->cb_client_bind_ip->clear();
        if (!m_ips.isEmpty()){
            if (m_ips.contains(text)){
                QStringList ds = m_ips.value(text);
                if (ds.length()>0){
                    ui->cb_client_bind_ip->addItems(ds);
                }
            }
        }
    }
}
