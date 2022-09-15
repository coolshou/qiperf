#include "formoption.h"
#include "ui_formoption.h"

FormOption::FormOption(QSettings *cfg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormOption)
{
    ui->setupUi(this);
    m_cfg = cfg;
    loadcfg(cfg);
}

FormOption::~FormOption()
{
    delete ui;
}

void FormOption::loadcfg(QSettings *cfg)
{
    //load cfg to ui
    cfg->beginGroup("test");
    ui->sb_parallel->setValue(cfg->value("parallel", 1).toInt());
    ui->sb_omit->setValue(cfg->value("omit", 3).toInt());
    ui->sb_time->setValue(cfg->value("time", 10).toInt());
    ui->sb_windowsize->setValue(cfg->value("windowsize", 2).toInt());
    int idx = ui->cb_windowsizeunit->findText(cfg->value("windowsizeunit", "M").toString());
    ui->cb_windowsizeunit->setCurrentIndex(idx);
    cfg->endGroup();
}

void FormOption::updatecfg()
{
    //save ui value to cfg
    m_cfg->beginGroup("test");
    m_cfg->setValue("parallel", ui->sb_parallel->value());
    m_cfg->setValue("omit", ui->sb_omit->value());
    m_cfg->setValue("time", ui->sb_time->value());
    m_cfg->setValue("windowsize", ui->sb_windowsize->value());
    m_cfg->setValue("windowsizeunit", ui->cb_windowsizeunit->currentText());
    QString args;
    args = "-P " + QString(ui->sb_parallel->value());
    args =args + " -O " + QString(ui->sb_omit->value());
    args =args + " -t " + QString(ui->sb_time->value());
    args =args + " -w " + QString(ui->sb_windowsize->value()) + ui->cb_windowsizeunit->currentText();
    m_cfg->setValue("args", args);
    m_cfg->endGroup();
}

void FormOption::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void FormOption::on_pb_cancel_clicked()
{
    //
    this->close();
}


void FormOption::on_pb_save_clicked()
{
    updatecfg();
    this->close();
}

