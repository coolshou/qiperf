#include "dlgset.h"
#include "ui_dlgset.h"

DlgSet::DlgSet(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgSet)
{
    ui->setupUi(this);
    connect(this, &DlgSet::accepted, this, &DlgSet::onAccepted);
}

DlgSet::~DlgSet()
{
    delete ui;
}

void DlgSet::setSSH(QString username, QString password)
{
    ui->leSSHUsername->setText(username);
    ui->leSSHPassword->setText(password);
}

void DlgSet::setWeb(QString username, QString password)
{
    ui->leWebUsername->setText(username);
    ui->leWebPassword->setText(password);

}

void DlgSet::changeEvent(QEvent *e)
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

void DlgSet::onAccepted()
{
    ControlBy ctl = ControlBy::None;
    //TODO: check text before send

    if (ui->cbControlBySSH->isChecked()){
        ctl = ControlBy::SSH;
    }
    if (ui->cbControlByQIperfd->isChecked()){
        ctl = ControlBy::QIPERFD;
    }
    emit updateSetting(ui->leSSHUsername->text(), ui->leSSHPassword->text(),
                       ui->leWebUsername->text(), ui->leWebPassword->text(),
                       ctl);
}

