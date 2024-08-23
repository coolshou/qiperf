#include "formqiperfds.h"
#include "ui_formqiperfds.h"

FormQIperfds::FormQIperfds(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormQIperfds)
{
    ui->setupUi(this);
    //setColumnWidth(0, 160);
    //setColumnWidth(1, 180);
    // Create the proxy model for sorting
    proxyModel = new QSortFilterProxyModel(this);
    // Ensure that the user can click on the header to sort
    ui->treeView->header()->setSortIndicatorShown(true);   // Show the sort indicator
    ui->treeView->header()->setSectionsClickable(true);    // Make headers clickable
}

FormQIperfds::~FormQIperfds()
{
    delete ui;
}

void FormQIperfds::setModel(QAbstractItemModel *model)
{
    proxyModel->setSourceModel(model);
    ui->treeView->setModel(proxyModel);
    ui->treeView->setSortingEnabled(true);
    // Optionally, initially sort by the first column
    ui->treeView->sortByColumn(0, Qt::AscendingOrder);
//    ui->treeView->setModel(model);
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
