#include "dlgrecord.h"
#include "ui_dlgrecord.h"

DlgRecord::DlgRecord(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgRecord)
{
    ui->setupUi(this);
}

DlgRecord::~DlgRecord()
{
    delete ui;
}

void DlgRecord::changeEvent(QEvent *e)
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
