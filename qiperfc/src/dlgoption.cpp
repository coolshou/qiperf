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
    connect(ui->sb_width_tp, QOverload<int>::of(&QSpinBox::valueChanged), this, &dlgOption::onWidthChange);
    connect(ui->sb_heigth_tp, QOverload<int>::of(&QSpinBox::valueChanged), this, &dlgOption::onHeigthChange);
    connect(ui->cb_TPGroup, QOverload<int>::of(&QCheckBox::stateChanged), this, &dlgOption::onStateChanged);
    connect(ui->cb_IgnoreWrongInterval, QOverload<int>::of(&QCheckBox::stateChanged), this, &dlgOption::onIgnoreWrongIntervalChanged);

    ui->tabWidget->setCurrentIndex(0);
}

dlgOption::~dlgOption()
{
    delete ui;
}

void dlgOption::loadcfg(QSettings *cfg)
{
    //load cfg to ui
    cfg->beginGroup("Iperf");
    ui->sb_WaitServerReady->setValue(cfg->value("WaitServerReady", 10).toInt());
    ui->sb_width_tp->setValue(cfg->value("TPExportWidth", 1280).toInt());
    ui->sb_heigth_tp->setValue(cfg->value("TPExportHeigth", 500).toInt());
    ui->cb_TPGroup->setChecked(cfg->value("TPGroup", false).toBool());
    ui->cb_IgnoreWrongInterval->setChecked(cfg->value("IgnoreWrongInterval", false).toBool());
    cfg->endGroup();

    cfg->beginGroup("agent");
    int midx = ui->cb_minterfaces->findText(cfg->value("managerifname", "").toString());
    if (midx>=0){
        ui->cb_minterfaces->setCurrentIndex(midx);
    }
    ui->sb_port->setValue(cfg->value("managerport", 45454).toInt());
    cfg->endGroup();

    cfg->beginGroup("notice");
    ui->cb_showManagerIPWarning->setChecked(cfg->value("showManagerIPWarning", true).toBool());
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
    m_cfg->setValue("TPGroup", ui->cb_TPGroup->isChecked());
    m_cfg->setValue("IgnoreWrongInterval", ui->cb_IgnoreWrongInterval->isChecked());
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

    m_cfg->beginGroup("notice");
    m_cfg->setValue("showManagerIPWarning", ui->cb_showManagerIPWarning->isChecked());
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

bool dlgOption::getShowManagerIPWarning()
{
    return ui->cb_showManagerIPWarning->isChecked();
}

// void dlgOption::setTPsize(int width, int heigth)
// {
//     // ui->sb_width_tp->setValue(width);
//     // ui->sb_heigth_tp->setValue(heigth);
// }

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

void dlgOption::onWidthChange(int width)
{
    emit widthChanged(width);
}

void dlgOption::onHeigthChange(int heigth)
{
    emit heigthChanged(heigth);
}

void dlgOption::onStateChanged(int state)
{
    if (state == Qt::Checked){
        emit showGroup(true);
        m_cfg->setValue("Iperf/TPGroup", true);
    }else{
        emit showGroup(false);
        m_cfg->setValue("Iperf/TPGroup", false);
    }
}

void dlgOption::onIgnoreWrongIntervalChanged(int state)
{
    if (state == Qt::Checked){
        emit IgnoreWrongInterval(true);
        m_cfg->setValue("Iperf/IgnoreWrongInterval", true);
    }else{
        emit IgnoreWrongInterval(false);
        m_cfg->setValue("Iperf/IgnoreWrongInterval", false);
    }
}

