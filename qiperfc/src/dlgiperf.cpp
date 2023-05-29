#include "dlgiperf.h"
#include "ui_dlgiperf.h"

DlgIperf::DlgIperf(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgIperf)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

}

DlgIperf::~DlgIperf()
{
    delete ui;
}

QString DlgIperf::getJsonCfg()
{
    //return Json config of iperf pair
    //server
    //client

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
