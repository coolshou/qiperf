#include "dlgssh.h"
#include "ui_dlgssh.h"

DlgSSH::DlgSSH(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgSSH)
{
    ui->setupUi(this);
}

DlgSSH::~DlgSSH()
{
    delete ui;
}

QString DlgSSH::getManagerIP()
{
    if (ui->cbManager->currentText().isEmpty()){
        return "127.0.0.1";
    }else{
        return ui->cbManager->currentText();
    }
}

QString DlgSSH::getTargetip()
{
    return ui->sshserver->currentText();
}

int DlgSSH::getTargetport()
{
    return ui->sshport->value();
}

QString DlgSSH::getSSHCfg()
{
    return QString("%1:%2:%3:%4:%5").arg(ui->leUsername->text(),
                                         ui->lePassword->text(),
                                         ui->lePromptLogin->text(),
                                         ui->lePromptPassword->text(),
                                         ui->lePromptReady->text());
}

QString DlgSSH::getLogFilename()
{
    if (ui->gbLogtoFile->isChecked()){
        return ui->leLogFilename->text();
    }else{
        return "";
    }
}

QString DlgSSH::getLogTimeStempFormat()
{
    if (ui->gbAddTimeStemp->isChecked()){
        return ui->leTimeStemp->text();
    } else {
        return "";
    }
}

void DlgSSH::changeEvent(QEvent *e)
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
