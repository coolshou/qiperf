#include "dlglbr.h"
#include "ui_dlglbr.h"

DlgLBR::DlgLBR(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgLBR)
{
    ui->setupUi(this);
}

DlgLBR::~DlgLBR()
{
    delete ui;
}

void DlgLBR::changeEvent(QEvent *e)
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
