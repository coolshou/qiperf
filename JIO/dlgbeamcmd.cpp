#include "dlgbeamcmd.h"
#include "ui_dlgbeamcmd.h"

DlgBeamCmd::DlgBeamCmd(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgBeamCmd)
{
    ui->setupUi(this);
}

DlgBeamCmd::~DlgBeamCmd()
{
    delete ui;
}

void DlgBeamCmd::clear()
{
    ui->textEdit->clear();
}

void DlgBeamCmd::onAddBeamIDCmd(QString cmd)
{
    ui->textEdit->append(cmd);
}

void DlgBeamCmd::changeEvent(QEvent *e)
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
