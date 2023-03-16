#include "dlgiperf.h"
#include "ui_dlgiperf.h"

DlgIperf::DlgIperf(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgIperf)
{
    ui->setupUi(this);
}

DlgIperf::~DlgIperf()
{
    delete ui;
}

void DlgIperf::changeEvent(QEvent *e)
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
