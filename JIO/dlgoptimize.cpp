#include "dlgoptimize.h"
#include "ui_dlgoptimize.h"

#include <QDebug>
#include "../src/numberdelegate.h"

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
    NumberDelegate *dDelegate = new NumberDelegate(NumberDelegate::Double,
                                                      0.0, 9999.0, 6,
                                                      ui->treeView);
    ui->treeView->setItemDelegateForColumn(OptimizeColumn::UL, dDelegate);
    ui->treeView->setItemDelegateForColumn(OptimizeColumn::DL, dDelegate);
    //test data
    // testdata1();
    // testdata2();
    // testdata3();
    // testdata4();

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

void DlgOptimize::testdata1()
{
    OptimizeData newTest(QDateTime::currentDateTime());
    QModelIndex midx = model->addEntry(newTest);  // Adds under root
    OptimizeData newRec1(QDateTime(), 99, 11, -45.4, 20, "CM7-1", 21, 11, -46, 23 , 990, 965);
    model->addEntry(newRec1, midx);  // Adds under root
    OptimizeData newRec2(QDateTime(), 99, 11, -47.4, 20, "CM7-2", 0, 10, -48, 22, 924, 913);
    model->addEntry(newRec2, midx);  // Adds under root
    OptimizeData newRec3(QDateTime(), 100, 11, -49, 20, "CM7-3", 43, 10, -49, 23, 914, 901);
    model->addEntry(newRec3, midx);  // Adds under root
    OptimizeData newRec4(QDateTime(), 100, 11, -47, 20, "CM7-4", 42, 11, -46, 23, 965, 954);
    model->addEntry(newRec4, midx);  // Adds under root
}

void DlgOptimize::testdata2()
{
    OptimizeData newTest(QDateTime::currentDateTime().addSecs(60));
    QModelIndex midx = model->addEntry(newTest);  // Adds under root
    OptimizeData newRec1(QDateTime(), 99, 11, -45.4, 20, "CM7-1", 23, 11, -45, 23, 967, 945);
    model->addEntry(newRec1, midx);  // Adds under root

}

void DlgOptimize::testdata3()
{
    OptimizeData newTest(QDateTime::currentDateTime().addSecs(120));
    QModelIndex midx = model->addEntry(newTest);  // Adds under root
    OptimizeData newRec1(QDateTime(), 99, 11, -45.4, 20, "CM7-1", 1, 11, -48, 21, 956, 934);
    model->addEntry(newRec1, midx);  // Adds under root

}
void DlgOptimize::testdata4()
{
    OptimizeData newTest(QDateTime::currentDateTime().addSecs(180));
    QModelIndex midx = model->addEntry(newTest);  // Adds under root
    OptimizeData newRec1(QDateTime(), 99, 11, -45.4, 20, "CM7-1", 43, 11, -48, 21);
    model->addEntry(newRec1, midx);  // Adds under root

}
