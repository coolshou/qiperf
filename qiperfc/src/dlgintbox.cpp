#include "dlgintbox.h"
#include "ui_dlgintbox.h"

DlgIntBox::DlgIntBox(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgIntBox)
{
    ui->setupUi(this);
}

DlgIntBox::~DlgIntBox()
{
    delete ui;
}

int DlgIntBox::getValue()
{
    return ui->sbDebugLevel->value();
}

void DlgIntBox::changeEvent(QEvent *e)
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
