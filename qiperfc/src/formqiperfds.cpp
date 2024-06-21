#include "formqiperfds.h"
#include "ui_formqiperfds.h"

FormQIperfds::FormQIperfds(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormQIperfds)
{
    ui->setupUi(this);
    //setColumnWidth(0, 160);
    //setColumnWidth(1, 180);

}

FormQIperfds::~FormQIperfds()
{
    delete ui;
}

void FormQIperfds::setModel(QAbstractItemModel *model)
{
    ui->treeView->setModel(model);
}

void FormQIperfds::setColumnWidth(int column, int width)
{
    ui->treeView->setColumnWidth(column, width);
}

void FormQIperfds::changeEvent(QEvent *e)
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
