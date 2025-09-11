#include "dlgoptimize.h"
#include "ui_dlgoptimize.h"

#include <QDebug>

DlgOptimize::DlgOptimize(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgOptimize)
{
    ui->setupUi(this);

    model= new OptimizeModel(ui->treeView);
    ui->treeView->setModel(model);
    ui->treeView->setHeaderHidden(false);           // Show headers
    ui->treeView->setRootIsDecorated(true);         // Show expand/collapse arrows
    ui->treeView->expandAll();                      // Optional: expand all nodes
    ui->treeView->setAlternatingRowColors(true);    // Optional: visual polish
    ui->treeView->show();

    ui->treeView->setColumnWidth(OptimizeColumn::TESTDATE, 140);
    ui->treeView->setColumnWidth(OptimizeColumn::ID, 50);
    ui->treeView->setColumnWidth(OptimizeColumn::MCS, 50);
    ui->treeView->setColumnWidth(OptimizeColumn::RSSI, 50);
    ui->treeView->setColumnWidth(OptimizeColumn::SNR, 50);
    // ui->treeView->setColumnWidth(OptimizeColumn::CM_NAME, 80);
    ui->treeView->setColumnWidth(OptimizeColumn::CM_ID, 50);
    ui->treeView->setColumnWidth(OptimizeColumn::CM_MCS, 50);
    ui->treeView->setColumnWidth(OptimizeColumn::CM_RSSI, 50);
    ui->treeView->setColumnWidth(OptimizeColumn::CM_SNR, 50);

    //test data
    // model
    OptimizeData newTest(QDateTime::currentDateTime());
    QModelIndex midx = model->addEntry(newTest);  // Adds under root
    OptimizeData newRec(QDateTime(), 99, 11, -45.4, 20, "CM1", 0, 0.0, 0.0, 0.0);
    model->addEntry(newRec, midx);  // Adds under root

    // qDebug() << "rowcount:" << model->rowCount(QModelIndex());     // Should be > 0
    // qDebug() << "columnCount:" << model->columnCount(QModelIndex());  // Should match your header count
}

DlgOptimize::~DlgOptimize()
{
    delete ui;
}

void DlgOptimize::changeEvent(QEvent *e)
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
