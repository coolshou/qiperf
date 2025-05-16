#include "sshview.h"
#include "ui_sshview.h"

SSHView::SSHView(QWidget *parent)
    : AbstractView(parent)
    , ui(new Ui::SSHView)
{
    ui->setupUi(this);
}

SSHView::~SSHView()
{
    delete ui;
}

void SSHView::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
