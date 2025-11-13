#include "dlgoptimize.h"
#include "ui_dlgoptimize.h"

#include <QDebug>
#include "../src/numberdelegate.h"

DlgOptimize::DlgOptimize(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgOptimize)
{
    ui->setupUi(this);

    model= new OptimizeModel();
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
    ui->treeView->setColumnWidth(OptimizeColumn::C_ID, 50);
    ui->treeView->setColumnWidth(OptimizeColumn::C_MCS, 50);
    ui->treeView->setColumnWidth(OptimizeColumn::C_RSSI, 50);
    ui->treeView->setColumnWidth(OptimizeColumn::C_SNR, 50);
    NumberDelegate *dDelegate = new NumberDelegate(NumberDelegate::Double,
                                                      0.0, 9999.0, 6,
                                                      ui->treeView);
    ui->treeView->setItemDelegateForColumn(OptimizeColumn::UL, dDelegate);
    ui->treeView->setItemDelegateForColumn(OptimizeColumn::DL, dDelegate);
    //test data
    // testdata1();

    // qDebug() << "rowcount:" << model->rowCount(QModelIndex());     // Should be > 0
    // qDebug() << "columnCount:" << model->columnCount(QModelIndex());  // Should match your header count
}

DlgOptimize::~DlgOptimize()
{
    delete ui;
}

void DlgOptimize::onAddData(QDateTime testtime, int apbeamid,
                            QString clientname, int clientbeamid)
{
    QModelIndex midx = model->findEntry(testtime);
    if (!midx.isValid()){
        OptimizeData newTest(testtime);
        midx = model->addEntry(newTest);  // Adds under root
    }
    // qDebug() << "apbeamid:" << apbeamid << " clientbeamid:" << clientbeamid;
    OptimizeData newRec1(QDateTime(), apbeamid, clientname, clientbeamid);
    QModelIndex newidx = model->addEntry(newRec1, midx);  // Adds under testtime item
    if (newidx.isValid()){
        ui->treeView->expand(midx);
    }

}

void DlgOptimize::onUpdateSignal(QDateTime testtime,
                                 double apmcs, double aprssi, double apsnr,
                                 QString cname,
                                 double cmcs, double crssi, double csnr)
{
    qDebug() << "TODO: onUpdateSignal ";
}

void DlgOptimize::onUpdateTP(QDateTime testtime, QString cname, double ul, double dl)
{
    qDebug() << "TODO: onUpdateTP ";
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
    OptimizeData newRec1(QDateTime(), 477, 11, -45.4, 20, "Client-1", 64, 11, -46, 23 , 990, 965);
    model->addEntry(newRec1, midx);  // Adds under root
    OptimizeData newRec2(QDateTime(), 477, 11, -47.4, 20, "Client-2", 66, 10, -48, 22, 924, 913);
    model->addEntry(newRec2, midx);  // Adds under root
    OptimizeData newRec3(QDateTime(), 476, 11, -49, 20, "Client-3", 64, 10, -49, 23, 914, 901);
    model->addEntry(newRec3, midx);  // Adds under root
    OptimizeData newRec4(QDateTime(), 476, 11, -47, 20, "Client-4", 64, 11, -46, 23, 965, 954);
    model->addEntry(newRec4, midx);  // Adds under root
}

