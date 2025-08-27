#include "dlgcyntec.h"
#include "ui_dlgcyntec.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QFileInfo>

#include "comm.h"

DlgCyntec::DlgCyntec(QSettings *cfg, Cyntec *cyntec, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgCyntec), m_cfg(cfg), mCyntec(cyntec)
{
    ui->setupUi(this);
    connect(ui->pbSelReffile, &QPushButton::clicked, this, &DlgCyntec::onSelReffileClicked);
    //Cyntec
    connect(ui->CyntecBeamFactorID, &QComboBox::currentTextChanged, this, &DlgCyntec::onCyntecBeamFactorIDChanged);
    connect(ui->CyntecBeamTableID, &QComboBox::currentTextChanged, this, &DlgCyntec::onCyntecBeamTableIDChanged);
    connect(ui->CyntecElementMap, &QComboBox::currentTextChanged, this, &DlgCyntec::onCyntecElementMapChanged);
    connect(ui->pbCyntecBeamTable, &QPushButton::clicked, this, &DlgCyntec::onCyntecBeamTableClicked);
    loadcfg();
}

DlgCyntec::~DlgCyntec()
{
    delete ui;
}
void DlgCyntec::onSelReffileClicked(bool checked)
{
    Q_UNUSED(checked)
    //select file
    QString path;
    if (!m_oldsavepath.isNull()){
        path = m_oldsavepath;
    }else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Load AIP module data file"),
                                                    path ,
                                                    tr(QIPERF_EXT_EXCEL));
    if (!fileName.isEmpty()){
        setRefFileName(fileName);
        onRefFileTextChanged(fileName);
        QFileInfo fileInfo(fileName);
        m_oldsavepath = fileInfo.path();
    }
}

void DlgCyntec::onRefFileTextChanged(QString newtext)
{
    emit reffilechanged(newtext);
}

void DlgCyntec::setRefFileName(QString filename)
{
    ui->lbRefFile->setText(filename);
}

void DlgCyntec::onNewCyntecBeamFactorIDs(QStringList keys)
{
    // qDebug() << "onNewCyntecBeamFactorIDs:" << keys;
    ui->CyntecBeamFactorID->clear();
    ui->CyntecBeamFactorID->insertItems(0, keys);
}

void DlgCyntec::onNewCyntecBeamTableIDs(QStringList keys)
{
    // qDebug() << "onNewCyntecBeamTableIDs:" << keys;
    ui->CyntecBeamTableID->clear();
    ui->CyntecBeamTableID->insertItems(0, keys);
}

void DlgCyntec::onUpdateCynteBeamFactorData(QString elementMap, int attDb, double azBW, double elBW)
{
    int idx = ui->CyntecElementMap->findText(elementMap);
    if (idx != ui->CyntecElementMap->currentIndex()){
        ui->CyntecElementMap->setCurrentIndex(idx);
    }
    ui->CyntecATT->setValue(attDb);
    ui->CyntecAzimuthBW->setValue(azBW);
    ui->CyntecElevationBW->setValue(elBW);
}

void DlgCyntec::onUpdateCyntecBeamTableData(double az, double el, double azBW, double elBW)
{
    Q_UNUSED(azBW)
    Q_UNUSED(elBW)
    ui->CyntecAZ->setValue(az);
    ui->CyntecEL->setValue(el);
}

void DlgCyntec::onCyntecBeamTableClicked(bool checked)
{
    Q_UNUSED(checked)
    if (mCyntec){
        FrmBeamTable *cBeamT = new FrmBeamTable(AIP::ModuleType::Cyntec);
        connect(this, &DlgCyntec::finished, cBeamT, &FrmBeamTable::close);
        connect(ui->CyntecBeamTableID, &QComboBox::currentTextChanged,
                cBeamT, &FrmBeamTable::selectEllipse);
        cBeamT->setGridPoints(mCyntec->getBeamTableDatas());
        cBeamT->show();
    }
}

void DlgCyntec::changeEvent(QEvent *e)
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
void DlgCyntec::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    savecfg();
}
void DlgCyntec::onCyntecBeamFactorIDChanged(QString newBeamFactorID)
{
    getCyntecBeamFactorDatas(newBeamFactorID);
}
void DlgCyntec::getCyntecBeamFactorDatas(QString beamFactorID)
{
    if(mCyntec){
        mCyntec->getBeamFactorDatas(beamFactorID.toInt());
    }else {
        qDebug() << "mCyntec not init";
    }
}
void DlgCyntec::onCyntecBeamTableIDChanged(QString newBeamTableID)
{
    if (!newBeamTableID.isEmpty()){
        if (mCyntec){
            mCyntec->getBeamTableData(newBeamTableID.toInt());
        }
    }
}
void DlgCyntec::onCyntecElementMapChanged(QString newElementMap)
{
    if (newElementMap.startsWith("8x8")){
        for (int i=0;i<8;i++){
            for (int j=0;j<8;j++){
                QTableWidgetItem *item = new QTableWidgetItem("O");
                item->setBackground(QBrush(Qt::green));
                ui->CyntecElementMapView->setItem(i,j, item);
            }
        }
    }else if (newElementMap.startsWith("8x4")){
        for (int i=0;i<8;i++){
            for (int j=0;j<8;j++){
                QTableWidgetItem *item = new QTableWidgetItem();
                if (j==0||j==1||j==6||j==7){
                    item->setText("X");
                    item->setBackground(QBrush(Qt::gray));
                }else{
                    item->setText("O");
                    item->setBackground(QBrush(Qt::green));
                }
                ui->CyntecElementMapView->setItem(i,j, item);
            }
        }
    }else if (newElementMap.startsWith("8x2")){
        for (int i=0;i<8;i++){
            for (int j=0;j<8;j++){
                QTableWidgetItem *item = new QTableWidgetItem();
                if (j==0||j==1||j==2||j==5||j==6||j==7){
                    item->setText("X");
                    item->setBackground(QBrush(Qt::gray));
                }else{
                    item->setText("O");
                    item->setBackground(QBrush(Qt::green));
                }
                ui->CyntecElementMapView->setItem(i,j, item);
            }
        }
    }else if (newElementMap.startsWith("4x4")){
        for (int i=0;i<8;i++){
            for (int j=0;j<8;j++){
                QTableWidgetItem *item = new QTableWidgetItem();
                if (i==0||i==1||i==6||i==7){
                    item->setText("X");
                    item->setBackground(QBrush(Qt::gray));
                }else{
                    if (j==0||j==1||j==6||j==7){
                        item->setText("X");
                        item->setBackground(QBrush(Qt::gray));
                    }else{
                        item->setText("O");
                        item->setBackground(QBrush(Qt::green));
                    }
                }
                ui->CyntecElementMapView->setItem(i,j, item);
            }
        }
    }else {
        for (int i=0;i<8;i++){
            for (int j=0;j<8;j++){
                QTableWidgetItem *item = new QTableWidgetItem("X");
                item->setBackground(QBrush(Qt::gray));
                ui->CyntecElementMapView->setItem(i,j, item);
            }
        }
    }
}
void DlgCyntec::getCyntecBeamTableDatas(QString beamTableID)
{
    if(mCyntec){
        mCyntec->getBeamTableData(beamTableID.toInt());
    }else{
        qDebug() << "mCyntec not init";
    }
}
void DlgCyntec::loadcfg()
{
    m_cfg->beginGroup("AIP");
    m_oldsavepath = m_cfg->value("selrefpath", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    m_cfg->endGroup();

    // ui->CyntecElementMap->setCurrentText(0);
    // onHanwhaBeamTypeTextChanged(ui->HanwhaBeamType->currentText());
}

void DlgCyntec::savecfg()
{
    m_cfg->beginGroup("AIP");
    m_cfg->setValue("selrefpath", m_oldsavepath);
    m_cfg->endGroup();
}

