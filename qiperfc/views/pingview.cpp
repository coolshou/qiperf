#include "pingview.h"
#include "ui_pingview.h"

PingView::PingView(QWidget *parent)
    : AbstractView(parent)
    , ui(new Ui::PingView)
{
    ui->setupUi(this);
}

PingView::~PingView()
{
    delete ui;
}

void PingView::changeEvent(QEvent *e)
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
