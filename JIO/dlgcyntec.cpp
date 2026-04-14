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
    nBeamT = nullptr;
    sBeamT = nullptr;
    tBeamT = nullptr;
    connect(ui->pbSelReffile, &QPushButton::clicked, this, &DlgCyntec::onSelReffileClicked);
    //Cyntec
    connect(ui->CyntecBeamFactorID, &QComboBox::currentTextChanged, this, &DlgCyntec::onCyntecBeamFactorIDChanged);
    connect(ui->CyntecBeamTableID, &QComboBox::currentTextChanged, this, &DlgCyntec::onCyntecBeamTableIDChanged);
    connect(ui->CyntecElementMap, &QComboBox::currentTextChanged, this, &DlgCyntec::onCyntecElementMapChanged);
    connect(ui->pbCyntecBeamTable, &QPushButton::clicked, this, &DlgCyntec::onCyntecBeamTableClicked);
    connect(ui->CyntecBeamType, &QComboBox::currentTextChanged, this, &DlgCyntec::onCyntecBeamTypeTextChanged);
    connect(ui->pbAround, &QPushButton::clicked, this, &DlgCyntec::onSelectAroundID);
    connect(ui->pbTriangle, &QPushButton::clicked, this, &DlgCyntec::onAddTriangle);
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

FrmBeamTable *DlgCyntec::getBeamTable()
{
    QString beamtype = ui->CyntecBeamType->currentText();
    if (beamtype.contains("Narrow")){
        return nBeamT;
    }else if (beamtype.contains("Spoiled")){
        return sBeamT;
    }else {
        return tBeamT;
    }
}

void DlgCyntec::setBeamTable(FrmBeamTable *beamtable)
{
    QString beamtype = ui->CyntecBeamType->currentText();
    if (beamtype.contains("Narrow")){
        nBeamT=beamtable;
    }else if (beamtype.contains("Spoiled")){
        sBeamT=beamtable;
    }else {
        tBeamT=beamtable;
    }
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

void DlgCyntec::onUpdateBeamFactorSupport(QMap<QString, QList<int> > data)
{
    mCyntecBeamFactorSupport = data;
}

void DlgCyntec::onNewCyntecBeamFactorIDs(QStringList keys)
{
    // qDebug() << "onNewCyntecBeamFactorIDs:" << keys;
    ui->CyntecBeamFactorID->clear();
    ui->CyntecBeamFactorID->insertItems(0, keys);
    //init
    if (ui->CyntecBeamFactorID->findText("1")!=-1){
        ui->CyntecBeamFactorID->setCurrentText("1");
    }
}

void DlgCyntec::onNewCyntecBeamTableIDs(QStringList keys)
{
    // qDebug() << "onNewCyntecBeamTableIDs:" << keys;
    ui->CyntecBeamTableID->clear();
    ui->CyntecBeamTableID->insertItems(0, keys);
}

void DlgCyntec::onUpdateCynteBeamFactorData(QString elementMap, double attDb, double azBW, double elBW)
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
        QString beamtype = ui->CyntecBeamType->currentText();
        FrmBeamTable *beamtable;
        beamtable = getBeamTable();
        if (!beamtable){
            //different beamtype use diff FrmBeamTable window
            beamtable = new FrmBeamTable(AIP::ModuleType::Cyntec);
            connect(this, &DlgCyntec::closeall, beamtable, &FrmBeamTable::close);
            connect(this, &DlgCyntec::SelectEllipse, beamtable, &FrmBeamTable::onSelectEllipse);
            setBeamTable(beamtable);
        }
        if (!beamtable->windowTitle().contains(beamtype)){
            beamtable->setWindowTitle(beamtable->windowTitle()+"-"+beamtype);
        }
        beamtable->setGridPoints(mCyntec->getBeamTableDatas(beamtype));
        beamtable->activateWindow();
        beamtable->show();
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
    QString beamtype = ui->CyntecBeamType->currentText();
    updateCyntecBeamTableID(beamtype, newBeamFactorID);
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
        QString scolor = ui->cbIDColor->currentText();
        QColor color(scolor);
        bool clear = ui->cbClear->isChecked();
        emit SelectEllipse(newBeamTableID, clear, color);
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
    bool doClear=false;
    if (ui->cbClear->checkState()==Qt::CheckState::Checked){
        doClear = true;
    }
    QString sColor = ui->cbAroundColor->currentText();
    bool clear = ui->cbClear->isChecked();
    QColor color(sColor);
    int id = ui->CyntecBeamTableID->currentText().toInt();
    QString beamtype = ui->CyntecBeamType->currentText();
    int glimit = ui->sbAroundLimit->value();
    QVector<int> ds= mCyntec->findNearestNeighbors(id, beamtype, glimit);
    if (ds.length()>0){
        FrmBeamTable *beamtable;
        beamtable = getBeamTable();
        if (beamtable && doClear){
            beamtable->clearEllipseSelection();
        }
        foreach (int idx, ds){
            emit SelectEllipse(QString::number(idx), clear, color);
        }
    }

}

void DlgCyntec::onAddTriangle(bool checked)
{
    Q_UNUSED(checked)
    double x = ui->sbTriangleX->value();
    double y = ui->sbTriangleY->value();
    double s = ui->sbTriangleSize->value();
    AddTriangle(x, y, s);
}

void DlgCyntec::onCyntecBeamTypeTextChanged(QString newBeamType)
{
    QString beamfactorID = ui->CyntecBeamFactorID->currentText();
    updateCyntecBeamTableID(newBeamType, beamfactorID);

}
void DlgCyntec::getCyntecBeamTableDatas(QString beamTableID)
{
    if(mCyntec){
        mCyntec->getBeamTableData(beamTableID.toInt());
    }else{
        qDebug() << "mCyntec not init";
    }
}

void DlgCyntec::updateCyntecBeamTableID(QString beamType, QString beamFactorID)
{
    if (mCyntecBeamTypeGroup.contains(beamType)){
        if (beamFactorID!="0"){
            QList<int> fs = mCyntecBeamFactorSupport.value(beamFactorID);
            qDebug() << "beamFactorID:" << beamFactorID << " fs:" << fs;
            QList<int> dataList= mCyntecBeamTypeGroup.value(beamType);
            qDebug() << "beamType:" << beamType << " dataList:" << dataList;
            QList<QString> stringList;
            for (int value : dataList) {
                if (fs.contains(value)){
                    stringList << QString::number(value);
                }
            }

            ui->CyntecBeamTableID->clear();
            ui->CyntecBeamTableID->insertItems(0, stringList);
            ui->CyntecBeamTableID->setEnabled(true);
        }else{
            ui->CyntecBeamTableID->setEnabled(false);
        }
    }else{
        qDebug() << "updateCyntecBeamTableID: mCyntecBeamTypeGroup do not have BeamType:" << beamType;
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

void DlgCyntec::AddTriangle(double xpos, double ypos, double size)
{
    FrmBeamTable *beamtable;
    beamtable = getBeamTable();
    if (beamtable){
        beamtable->addTriangleTarget(QPointF(xpos, ypos), size);
    }
}

