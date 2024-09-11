#include "dlgoption.h"
#include "ui_dlgoption.h"

dlgOption::dlgOption(QSettings *cfg, QWidget *parent) :
//FormOption::FormOption(QSettings *cfg, QStringList interfaces, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgOption)
{
    ui->setupUi(this);
    m_cfg = cfg;
//    ui->cb_minterfaces->addItems(interfaces);
    loadcfg(cfg);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &dlgOption::onAccept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &dlgOption::onReject);

}

dlgOption::~dlgOption()
{
    delete ui;
}

void dlgOption::loadcfg(QSettings *cfg)
{
    //load cfg to ui
    cfg->beginGroup("iperf");
    cfg->endGroup();
    cfg->beginGroup("agent");
    int midx = ui->cb_minterfaces->findText(cfg->value("managerifname", "").toString());
    if (midx>=0){
        ui->cb_minterfaces->setCurrentIndex(midx);
    }
    ui->sb_port->setValue(cfg->value("managerport", 45454).toInt());
    cfg->endGroup();
}

void dlgOption::updatecfg()
{
    //save ui value to cfg
    m_cfg->beginGroup("Iperf");
    QString args;
    m_cfg->setValue("args", args);
    m_cfg->setValue("WaitServerReady", ui->sb_WaitServerReady->value());
    m_cfg->setValue("TPExportWidth", ui->sb_width_tp->value());
    m_cfg->setValue("TPExportHeigth", ui->sb_heigth_tp->value());
    m_cfg->endGroup();

    m_cfg->beginGroup("agent");
    QString ifname = ui->cb_minterfaces->currentText();
    m_cfg->setValue("managerifname", ifname);
    QStringList qs = ifname.split(": ");
    int port = ui->sb_port->value();
    if (qs.length()>=2){
        emit ipaddressUpdated(qs[1], port);
    }
    m_cfg->setValue("managerport", port);
    m_cfg->endGroup();
    m_cfg->sync();
}

void dlgOption::setWaitServerReady(int val)
{
    if ((val >= ui->sb_WaitServerReady->minimum()) &&
        (val <= ui->sb_WaitServerReady->maximum())){
        ui->sb_WaitServerReady->setValue(val);
    }
}

int dlgOption::getWaitServerReady()
{
    return ui->sb_WaitServerReady->value();
}

void dlgOption::setTPsize(int width, int heigth)
{
    // ui->sb_width_tp->setValue(width);
    // ui->sb_heigth_tp->setValue(heigth);
}

void dlgOption::changeEvent(QEvent *e)
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

void dlgOption::onReject()
{
    this->close();
}


void dlgOption::onAccept()
{
    updatecfg();
    this->close();
}

