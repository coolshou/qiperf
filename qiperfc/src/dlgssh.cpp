#include "dlgssh.h"
#include "ui_dlgssh.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QDir>
#include "comm.h"

DlgSSH::DlgSSH(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgSSH)
{
    ui->setupUi(this);
    connect(ui->pbPrivateKeyFile, &QPushButton::clicked, this, &DlgSSH::onSelectPrivateKeyFile);
    connect(ui->pbSelectLogFile, &QPushButton::clicked, this, &DlgSSH::onSelectLogFile);
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
    return QString("%1:%2:%3:%4").arg(ui->leUsername->text(),
                                      ui->lePassword->text(),
                                      getPrivateKeyFilename(),
                                      QString::number(ui->sshtimeout->value()));

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

QString DlgSSH::getPrivateKeyFilename()
{
    if (!ui->lePrivateKeyFile->text().isEmpty()){
        return ui->lePrivateKeyFile->text();
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

void DlgSSH::onSelectPrivateKeyFile(bool checked)
{
    Q_UNUSED(checked)
    if (oldsshpath.isEmpty()){
        oldsshpath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }
    QString path = oldsshpath + QDir::separator() + ".ssh";
    QString fileName = QFileDialog::getOpenFileName(this, tr("set Private Key filename"),
                                                    path, tr(ALL_EXT_FILTER));
    if (!fileName.isEmpty()){
        ui->lePrivateKeyFile->setText(fileName);
        QFileInfo fi(fileName);
        oldsshpath = fi.path();
    }
}

void DlgSSH::onSelectLogFile(bool checked)
{
    Q_UNUSED(checked)
    if (oldpath.isEmpty()){
        oldpath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString path = oldpath + QDir::separator() + "ssh.log";
    QString fileName = QFileDialog::getSaveFileName(this, tr("set log filename"),
                                                    path, tr(ALL_EXT_FILTER));
    if (!fileName.isEmpty()){
        ui->leLogFilename->setText(fileName);
        QFileInfo fi(fileName);
        oldpath = fi.path();
    }
}
