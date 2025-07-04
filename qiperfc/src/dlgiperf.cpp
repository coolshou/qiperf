#include "dlgiperf.h"
#include "ui_dlgiperf.h"
#include "../src/comm.h"
#include "../src/myfunc.h"

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

#include "../src/showcustomtooltip.h"

DlgIperf::DlgIperf(TPMgr *tpmgr, QSettings *cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgIperf)
{
    ui->setupUi(this);
    m_dlgrule = new DlgIperfRestartRule(cfg);
    ui->wManagement->setVisible(false);
    adjustSize();
    connect(ui->pbManagement, &QPushButton::clicked, this, &DlgIperf::showManagement);
    m_tpmgr = tpmgr;
    mgrls.append("");
    old_mss = 0;
    b_ipv6 = false;
    connect(ui->cb_version, &QComboBox::currentTextChanged, this, &DlgIperf::ChangeVersion);
#if QT_VERSION < QT_VERSION_CHECK(6,7,0)  // < 6.7
    //stateChanged (until 6.9)
    connect(ui->chk_bidir, &QCheckBox::stateChanged, this, &DlgIperf::onChkBidirStatech);
    connect(ui->chk_reverse, &QCheckBox::stateChanged, this, &DlgIperf::onChkReverseStatech);
    connect(ui->cbTimeStamp, &QCheckBox::stateChanged, this, &DlgIperf::onTimeStampstateChanged);
#else
    connect(ui->chk_bidir, &QCheckBox::checkStateChanged, this, &DlgIperf::onChkBidirStatech);
    connect(ui->chk_reverse, &QCheckBox::checkStateChanged, this, &DlgIperf::onChkReverseStatech);
    connect(ui->cbTimeStamp, &QCheckBox::checkStateChanged, this, &DlgIperf::onTimeStampstateChanged);
#endif
//    connect(ui, &QDialog::accepted, this, &QDialog::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DlgIperf::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

//    connect(ui->cb_mserver_ip, QOverload<int>::of(&QComboBox::currentIndexChanged),
//            this,&DlgIperf::onSelectMServer);
    connect(ui->cb_mserver_ip, &QComboBox::currentTextChanged, this,&DlgIperf::onSelectMServer);
    connect(ui->cb_mclient_ip, &QComboBox::currentTextChanged, this,&DlgIperf::onSelectMClient);
    connect(ui->sb_duration, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &DlgIperf::onDurationValueChanged);

    connect(ui->cb_fmtreport, &QComboBox::currentTextChanged, this, &DlgIperf::onFmtreportChanged);
#if QT_VERSION < QT_VERSION_CHECK(6,7,0)  // < 6.7
    connect(ui->cbRestartOnError, &QCheckBox::stateChanged, this, &DlgIperf::onChkRestartOnError);
#else
    connect(ui->cbRestartOnError, &QCheckBox::checkStateChanged, this, &DlgIperf::onChkRestartOnError);
#endif
    connect(ui->pbRestartRule, &QPushButton::clicked, this, &DlgIperf::onShowIperfRestartRule);
// lbDuration
    // connect(ui->sb_mss, &QSpinBox::valueChanged, this, &DlgIperf::onMSSvalueChanged); //TODO: Not good for UI interaction

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
    mainObj.insert("enabled", m_enabled);

    //server
    QJsonObject serverObj;
    serverObj.insert("version", ui->cb_version->currentText());
    serverObj.insert("port", ui->sb_port->value());
    if (ui->cb_mserver_ip->currentText().isEmpty()){
        serverObj.insert("manager", ui->cb_target_ip->currentText().trimmed());
    }else{
        serverObj.insert("manager", ui->cb_mserver_ip->currentText().trimmed());
    }
    serverObj.insert("protocal", ui->cb_protocal->currentText());
    serverObj.insert("parallel", ui->sb_parallel->value());
    serverObj.insert("windowsize", ui->sb_windowsize->value());
    serverObj.insert("unit_windowsize", ui->cb_unit_windowsize->currentText());
    serverObj.insert("reverse", ui->chk_reverse->isChecked());
    serverObj.insert("bidir", ui->chk_bidir->isChecked());
    serverObj.insert("interval", ui->sb_interval->value());
    //        serverObj.insert("ipv6", b_ipv6);
    if (ui->chk_server_bind_ip->isChecked()){
        serverObj.insert("bind", ui->cb_target_ip->currentText().trimmed());
    }
    serverObj.insert("fmtreport", ui->cb_fmtreport->currentText().trimmed());
    serverObj.insert("delaytime", ui->sb_delaytime->value());
    if (ui->cbTimeStamp->isChecked()){
        serverObj.insert("timestamps", ui->leTimeStamp->text().trimmed());
    }

    //client
    QJsonObject clientObj;
    clientObj.insert("version", ui->cb_version->currentText());
    clientObj.insert("port", ui->sb_port->value());
    if (ui->cb_mclient_ip->currentText().isEmpty()){
        clientObj.insert("manager", ui->cb_client_bind_ip->currentText().trimmed());
    }else{
        clientObj.insert("manager", ui->cb_mclient_ip->currentText().trimmed());
    }
    clientObj.insert("ipv6", b_ipv6);
    if (!ui->cb_client_bind_ip->currentText().isEmpty()) {
        //TODO: check IPv4/IPv6format
        clientObj.insert("bind", ui->cb_client_bind_ip->currentText().trimmed());
    }
    clientObj.insert("protocal", ui->cb_protocal->currentText());
    clientObj.insert("target", ui->cb_target_ip->currentText().trimmed());
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
    clientObj.insert("zerocopy", ui->cb_zerocopy->isChecked());
    clientObj.insert("delaytime", ui->sb_delaytime->value());
    if (ui->cbTimeStamp->isChecked()){
        clientObj.insert("timestamps", ui->leTimeStamp->text().trimmed());
    }

    //restart rule
    serverObj.insert("restartonerror", ui->cbRestartOnError->isChecked());
    clientObj.insert("restartonerror", ui->cbRestartOnError->isChecked());
    if (ui->cbRestartOnError->isChecked()){
        QJsonObject rule = m_dlgrule->getJsonCfgObj();
        serverObj.insert("restartrules", rule);
        clientObj.insert("restartrules", rule);
    }
    mainObj.insert("server", serverObj);
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
        m_enabled = mainObj["enabled"].toBool(true);
        QJsonObject serverObj = mainObj["server"].toObject();
        QJsonObject clientObj = mainObj["client"].toObject();
        bool bRestartonerror = serverObj["restartonerror"].toBool();
        ui->cbRestartOnError->setChecked(bRestartonerror);
        ui->pbRestartRule->setEnabled(bRestartonerror);
        if (bRestartonerror){
            QJsonObject rulesObj = serverObj["restartrules"].toObject();
            m_dlgrule->setJsonRules(rulesObj);
        }


        ui->cb_version->setCurrentText(serverObj["version"].toString());
        ui->sb_port->setValue(serverObj["port"].toInt());
        if ((!serverObj["manager"].toString().isEmpty()) &&
            (serverObj["manager"].toString() != clientObj["target"].toString())){
            ui->cb_mserver_ip->setCurrentText(serverObj["manager"].toString());
        }
        if (!serverObj["bind"].toString().isEmpty()){
            ui->chk_server_bind_ip->setChecked(true);
        }
        ui->cb_protocal->setCurrentText(serverObj["protocal"].toString());
        ui->sb_parallel->setValue(serverObj["parallel"].toInt());
        ui->chk_bidir->setChecked(serverObj["bidir"].toBool());
        ui->sb_interval->setValue(serverObj["interval"].toInt());

        ui->sb_delaytime->setValue(serverObj["delaytime"].toInt());
        ui->cbRestartOnError->setChecked(serverObj["restartonerror"].toBool());
        if (serverObj.contains("timestamps")){
            ui->cbTimeStamp->setChecked(true);
            ui->leTimeStamp->setText(serverObj["timestamps"].toString());
        }else{
            ui->cbTimeStamp->setChecked(false);
            ui->leTimeStamp->setText("%m%d%H%M%S");
        }
        // client
        if ((!clientObj["manager"].toString().isEmpty()) &&
            (clientObj["manager"].toString() != clientObj["bind"].toString())) {
            ui->cb_mclient_ip->setCurrentText(clientObj["manager"].toString());
        }
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
        ui->cb_zerocopy->setChecked(clientObj["zerocopy"].toBool());

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
                                        QJsonArray addrArray = addr.toArray();
                                        if (!addrArray.isEmpty()) {
                                            ds.append(addrArray[0].toString());
                                        }
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

bool DlgIperf::isRequireConfigMeet()
{
    QString targetip = ui->cb_target_ip->currentText().trimmed();
    //check require fields value
    QHostAddress addr_target;
    bool bok = addr_target.setAddress(targetip);
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
    // if (ui->cb_mserver_ip->currentText()==""){
    //     //TODO: use target ip as manager server ip
    // }
    // if (ui->cb_mclient_ip->currentText()==""){
    //     //TODO: use bind ip as manager client ip
    // }
    // if (ui->cb_mserver_ip->currentText() == ui->cb_mclient_ip->currentText()){
    //     QMessageBox::warning(this, tr("WARNING!!"),
    //                          tr("Forbid setting same address of Manager Server and Manager Client!!"),
    //                          QMessageBox::Ok);
    //     ui->cb_mclient_ip->setFocus();
    //     //TODO: use style to hightlight some item:  *{border: 3px solid red;}
    //     return false;
    // }
    /*
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
                                     tr("The parallel number should not over 20 (iperf3 under window may have problem)!!"),  QMessageBox::Ok);
                ui->sb_parallel->setValue(20);
                ui->sb_parallel->setFocus();
                return false;
            }
        }
    }*/


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
    QString bindkey = targetip+"_"+ QString::number(ui->sb_port->value());
    if (m_tpmgr->isBindkeyExist(ui->cb_mserver_ip->currentText().trimmed(), bindkey, m_excIdx))
    {
        QString msg = ui->cb_mserver_ip->currentText().trimmed() + " already have " + bindkey+ "\n Please use other value of port";
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
    // bandwidth unit
    QStringList unitBW;
    if (ver.indexOf("2")==0){
        unitBW << "k" << "m" << "g" << "K" << "M" << "G" << "pps";
    }else{
        unitBW << "K" << "M" << "G";
    }
    ui->cb_unit_bitrate->clear();
    ui->cb_unit_bitrate->addItems(unitBW);
    // length of buffer
    QStringList unitBuffer;
    if (ver.indexOf("2")==0){
        unitBuffer << "" << "k" << "m" << "K" << "M";
    }else{
        unitBuffer << "" << "K" << "M" << "G";
    }
    ui->cb_unit_buffer->clear();
    ui->cb_unit_buffer->addItems(unitBuffer);
    // window size
    QStringList unitWindowSize;
    if (ver.indexOf("2")==0){
        unitWindowSize << "K" << "M";
    }else{
        unitWindowSize << "K" << "M" << "G";
    }
    ui->cb_unit_windowsize->clear();
    ui->cb_unit_windowsize->addItems(unitWindowSize);
    // format unit
    QStringList unitFormat;
    if (ver.indexOf("2")==0){
        unitFormat << "k" << "m" << "g" << "K" << "M" << "G";
    }else{
        unitFormat << "k" << "m" << "g" << "t" << "K" << "M" << "G" << "T";
    }
    ui->cb_fmtreport->clear();
    ui->cb_fmtreport->addItems(unitFormat);
    ui->cb_fmtreport->setCurrentText("m");
    QString sBidir;
    if (ver.indexOf("2")==0){
        sBidir = "bidirectional(-d, --dualtest)";
    }else{
        sBidir = "bidirectional(--bidir)";
    }
    ui->chk_bidir->setText(sBidir);
    // omit
    if (ver.indexOf("2")==0){
        ui->lbOmit->setText("omit(--omit, sec(TCP only)):");
    }else{
        ui->lbOmit->setText("omit(-O, sec):");
    }
    //TODO: Type of service (TOS)
    if (ver.indexOf("2")==0){
        // ui->lb_tos
        // ui->sb_tos
        /* Accepted tos values are: af11, af12, af13, af21, af22, af23, af31, af32, af33, af41, af42, af43,
        cs0, cs1, cs2, cs3, cs4, cs5, cs6, cs7, ef, le, nqb, nqb2, ac_be, ac_bk, ac_vi, ac_vo, lowdelay, throughput,
            reliability, or a numeric value.
        */
    }else {
        // numeric value
        // ui->sb_tos
    }
    //timestemp
    if (ver.indexOf("2")==0){
        ui->wTimeStamp->setEnabled(false);
        ui->cbTimeStamp->setChecked(false);
    }else{
        ui->wTimeStamp->setEnabled(true);
        ui->leTimeStamp->setText("%m%d%H%M%S");
    }
}

void DlgIperf::onAccepted()
{
    if (isRequireConfigMeet()){
        accept();
    }else{
        return;
    }
}
#if QT_VERSION < QT_VERSION_CHECK(6,7,0)  // < 6.7
void DlgIperf::onChkBidirStatech(int state)
#else
void DlgIperf::onChkBidirStatech(Qt::CheckState state)
#endif
{
    if (state==Qt::Checked){
        ui->chk_reverse->setCheckState(Qt::Unchecked);
    }
}

#if QT_VERSION < QT_VERSION_CHECK(6,7,0)  // < 6.7
void DlgIperf::onChkReverseStatech(int state)
#else
void DlgIperf::onChkReverseStatech(Qt::CheckState state)
#endif
{
    if (state==Qt::Checked){
        ui->chk_bidir->setCheckState(Qt::Unchecked);
    }
}

#if QT_VERSION < QT_VERSION_CHECK(6,7,0)  // < 6.7
void DlgIperf::onTimeStampstateChanged(int state)
#else
void DlgIperf::onTimeStampstateChanged(Qt::CheckState state)
#endif
{
    if (state==Qt::Checked){
        ui->leTimeStamp->setEnabled(true);
    }else{
        ui->leTimeStamp->setEnabled(false);
    }
}
#if QT_VERSION < QT_VERSION_CHECK(6,7,0)  // < 6.7
void DlgIperf::onChkRestartOnError(int state)
#else
void DlgIperf::onChkRestartOnError(Qt::CheckState state)
#endif
{
    if (state==Qt::Checked){
        ui->pbRestartRule->setEnabled(true);
    }else{
        ui->pbRestartRule->setEnabled(false);
    }
}
// #else
// void DlgIperf::onChkBidirStatech(Qt::CheckState state)
// {
//     if (state==Qt::Checked){
//         ui->chk_reverse->setCheckState(Qt::Unchecked);
//     }
// }

// void DlgIperf::onChkReverseStatech(Qt::CheckState state)
// {
//     if (state==Qt::Checked){
//         ui->chk_bidir->setCheckState(Qt::Unchecked);
//     }
// }
// void DlgIperf::onTimeStampstateChanged(Qt::CheckState state)
// {
//     if (state==Qt::Checked){
//         ui->leTimeStamp->setEnabled(true);
//     }else{
//         ui->leTimeStamp->setEnabled(false);
//     }
// }
// #endif

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

void DlgIperf::onMSSvalueChanged(int value)
{
    if ((value<88)&&(value>0)){
        // TODO: the tooltip only show < 1 sec ? why?
        showCustomToolTip(ui->sb_mss, ui->sb_mss->toolTip());
        // qDebug() << "old_mss:" << QString::number(old_mss) << " new:" << QString::number(value);
        if (value> old_mss){
            ui->sb_mss->setValue(88);
            old_mss = 88;
        } else {
            ui->sb_mss->setValue(0);
            old_mss = 0;
        }
    }else{
        //store value as old_mss
        old_mss = value;
    }
}

void DlgIperf::showManagement(bool show)
{
    if (show){
        setMinimumSize(465,664);
    }
    ui->wManagement->setVisible(show);
    if (!show){
        setMinimumSize(465,586);
        resize(465, 586);
    }
}

void DlgIperf::onDurationValueChanged(int value)
{
    QString s="";
    if (value==0){
        s = "Run Forever";
    }else {
        s = MyFunc::secToHumanReadable(value);
    }
    ui->lbDuration->setText(s);
}

void DlgIperf::onFmtreportChanged(QString text)
{
    qDebug() << "onFmtreportChanged: " << text;
    // "k" << "m" << "g" << "t" << "K" << "M" << "G" << "T";
}

void DlgIperf::onShowIperfRestartRule(bool checked)
{
    Q_UNUSED(checked)
    m_dlgrule->exec();
}
