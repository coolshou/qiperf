#include "dlgserial.h"
#include "ui_dlgserial.h"

DlgSerial::DlgSerial(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgSerial)
{
    ui->setupUi(this);
}

DlgSerial::~DlgSerial()
{
    delete ui;
}

void DlgSerial::changeEvent(QEvent *e)
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
