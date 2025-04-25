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
