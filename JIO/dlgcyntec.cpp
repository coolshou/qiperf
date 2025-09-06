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
    connect(ui->CyntecBeamType, &QComboBox::currentTextChanged, this, &DlgCyntec::onCyntecBeamTypeTextChanged);
    connect(ui->pbAround, &QPushButton::clicked, this, &DlgCyntec::onSelectAroundID);
    loadcfg();
}

DlgCyntec::~DlgCyntec()
{
    delete ui;
}
QString DlgCyntec::getCyntecBeamType()
{
    return ui->CyntecBeamType->currentText();
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

void DlgCyntec::onUpdateBeamTypes(QStringList beamtypes)
{
    ui->CyntecBeamType->insertItems(0, beamtypes);
}

void DlgCyntec::onUpdateBeamTypeGroup(QMap<QString, QList<int>> data)
{
    mCyntecBeamTypeGroup = data;
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
        // connect(this, &DlgCyntec::finished, cBeamT, &FrmBeamTable::close);
        connect(this, &DlgCyntec::closeall, cBeamT, &FrmBeamTable::close);
        //TODO: CyntecBeamTableID change
        connect(ui->CyntecBeamTableID, &QComboBox::currentTextChanged,
                cBeamT, &FrmBeamTable::onSelectEllipse);
        connect(this, &DlgCyntec::SelectEllipse, cBeamT, &FrmBeamTable::onSelectEllipse);
        QString beamtype = ui->CyntecBeamType->currentText();
        cBeamT->setWindowTitle(cBeamT->windowTitle()+"-"+beamtype);
        cBeamT->setGridPoints(mCyntec->getBeamTableDatas(beamtype));
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
    emit closeall();
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

void DlgCyntec::onSelectAroundID(bool checked)
{
    Q_UNUSED(checked)
    int id = ui->CyntecBeamTableID->currentText().toInt();
    QString beamtype = ui->CyntecBeamType->currentText();
    QVector<int> ds= mCyntec->findNearestNeighbors(id, beamtype);
    if (ds.length()>0){
        foreach (int idx, ds){
            emit SelectEllipse(QString::number(idx), false);
        }
    }

}

void DlgCyntec::onCyntecBeamTypeTextChanged(QString newBeamType)
{
    if (mCyntecBeamTypeGroup.contains(newBeamType)){
        QList<int> dataList= mCyntecBeamTypeGroup.value(newBeamType);
        QList<QString> stringList;
        for (int value : dataList) {
            stringList << QString::number(value);
        }

        ui->CyntecBeamTableID->clear();
        ui->CyntecBeamTableID->insertItems(0, stringList);
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

