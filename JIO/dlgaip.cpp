#include "dlgaip.h"
#include "ui_dlgaip.h"

#include <QFile>
#include <QStandardPaths>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QDebug>


#include "comm.h"

DlgAIP::DlgAIP(QSettings *cfg, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgAIP), m_cfg(cfg)
{
    mPosOffset=QVector3D(0.0,0.0,10.0);
    ui->setupUi(this);
    connect(this, &DlgAIP::accepted, this, &DlgAIP::onAccepted);
    loadcfg();
    mModuleType = AIP::ModuleType::Unknown;
    connect(ui->pbSelReffile, &QPushButton::clicked, this, &DlgAIP::onSelReffileClicked);
    connect(ui->cbAIPModule, &QComboBox::currentTextChanged, this, &DlgAIP::onChangeModule);
    connect(ui->leRefFile, &QLineEdit::textChanged, this, &DlgAIP::onRefFileTextChanged);
    //Cyntec
    connect(ui->CyntecBeamFactorID, &QComboBox::currentTextChanged, this, &DlgAIP::onCyntecBeamFactorIDChanged);
    connect(ui->CyntecElementMap, &QComboBox::currentTextChanged, this, &DlgAIP::onCyntecElementMapChanged);
    connect(ui->CyntecBeamTableID, &QComboBox::currentTextChanged, this, &DlgAIP::onCyntecBeamTableIDChanged);
    mCyntec = new Cyntec();
    connect(mCyntec, &Cyntec::newBeamFactorIDs, this, &DlgAIP::onNewCyntecBeamFactorIDs);
    connect(mCyntec, &Cyntec::newBeamTableIDs, this, &DlgAIP::onNewCyntecBeamTableIDs);
    connect(mCyntec, &Cyntec::updateBeamFactorData, this, &DlgAIP::onUpdateCynteBeamFactorData);
    connect(mCyntec, &Cyntec::updateBeamTableData, this, &DlgAIP::onUpdateBeamTableData);

    //Hanwha
    //load data ?
}

DlgAIP::~DlgAIP()
{
    delete ui;
}

AIP::ModuleType DlgAIP::getModuleType()
{
    return mModuleType;
}

void DlgAIP::loadData(QString sdata)
{
    //load json string data and show on UI
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(sdata.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        QJsonObject data= doc.object();
        loadData(data);
    }
}

void DlgAIP::loadData(QJsonObject data)
{
    //load json data and show on UI
    qDebug() << "TODO load Json data:" << data;
}

QJsonObject DlgAIP::getData()
{
    // get UI's value to turn into JSON format
    qDebug() << "TODO get Json data";
    QJsonObject jobj;
    jobj["moduletype"] = static_cast<int>(mModuleType);
    jobj["X_Offset"] = mPosOffset.x();
    jobj["Y_Offset"] = mPosOffset.y();
    jobj["Z_Offset"] = mPosOffset.z();
    return jobj;
}

void DlgAIP::setPosOffset(float xpos, float ypos, float zpos)
{
    mPosOffset.setX(xpos);
    mPosOffset.setY(ypos);
    mPosOffset.setZ(zpos);
}

void DlgAIP::setRowCol(int row, int col)
{
    mRow=row;
    mCol=col;
}

void DlgAIP::changeEvent(QEvent *e)
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

void DlgAIP::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    savecfg();
}

void DlgAIP::onAccepted()
{
    QJsonObject obj = getData();
    QJsonDocument doc(obj);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    emit updateData(mRow, mCol, strJson);
}

void DlgAIP::onSelReffileClicked(bool checked)
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
        ui->leRefFile->setText(fileName);
        QFileInfo fileInfo(fileName);
        m_oldsavepath = fileInfo.path();
    }
}

void DlgAIP::onChangeModule(QString newtext)
{
    qDebug() << "onChangeModule:" << newtext;
    int idxCyntec =ui->tabWidget->indexOf(ui->tabCyntec);
    int idxHanwha =ui->tabWidget->indexOf(ui->tabHanwha);
    if (newtext.startsWith("Cyntec")){
        ui->leRefFile->setText("Cyntec_beam_table_v0.2.5.xlsx");
        mModuleType = AIP::ModuleType::Cyntec;
        ui->tabWidget->setTabVisible(idxCyntec, true);
        ui->tabWidget->setTabVisible(idxHanwha, false);
    }else if (newtext.startsWith("Hanwha")){
        ui->leRefFile->setText("a41c_beam_table_export_v5.xlsx");
        mModuleType = AIP::ModuleType::Hanwha;
        ui->tabWidget->setTabVisible(idxCyntec, false);
        ui->tabWidget->setTabVisible(idxHanwha, true);
    }else{
        ui->tabWidget->setTabVisible(idxCyntec, false);
        ui->tabWidget->setTabVisible(idxHanwha, false);
    }
}

void DlgAIP::onRefFileTextChanged(QString newtext)
{
    QFile f(newtext);
    if (f.exists()){
        if(mModuleType == AIP::ModuleType::Cyntec){
            // read Cyntec's xls file
            mCyntec->initBeamData(newtext);

        }else if(mModuleType == AIP::ModuleType::Hanwha){
            qDebug() << "onRefFileTextChanged: read Hanwha xls file";
        }else {
            qDebug() << "onRefFileTextChanged: unknows module type";
        }
    }else {
        qDebug() << "File not exist: " << newtext;
        QString msg = QString("File not exist: %1").arg(newtext);
        QMessageBox::warning(this, tr("WARNING!!"),
                             msg,
                             QMessageBox::Ok);
        ui->leRefFile->setFocus();
    }

}

void DlgAIP::onCyntecBeamFactorIDChanged(QString newBeamFactorID)
{
    getCyntecBeamFactorDatas(newBeamFactorID);
}

void DlgAIP::onCyntecElementMapChanged(QString newElementMap)
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

void DlgAIP::onCyntecBeamTableIDChanged(QString newBeamTableID)
{

}

void DlgAIP::onNewCyntecBeamFactorIDs(QStringList keys)
{
    ui->CyntecBeamFactorID->clear();
    ui->CyntecBeamFactorID->insertItems(0, keys);
}

void DlgAIP::onNewCyntecBeamTableIDs(QStringList keys)
{
    ui->CyntecBeamTableID->clear();
    ui->CyntecBeamTableID->insertItems(0, keys);
}

void DlgAIP::onUpdateCynteBeamFactorData(QString elementMap, int attDb, double azBW, double elBW)
{
    int idx = ui->CyntecElementMap->findText(elementMap);
    if (idx){
        ui->CyntecElementMap->setCurrentIndex(idx);
    }
    ui->CyntecATT->setValue(attDb);
    ui->CyntecAzimuthBW->setValue(azBW);
    ui->CyntecElevationBW->setValue(elBW);
}

void DlgAIP::onUpdateBeamTableData(int az, int el, double azBW, double elBW)
{
    Q_UNUSED(azBW)
    Q_UNUSED(elBW)
    ui->CyntecAZ->setValue(az);
    ui->CyntecEL->setValue(el);
}

void DlgAIP::getCyntecBeamFactorDatas(QString beamFactorID)
{
    if(mCyntec){
        mCyntec->getBeamFactorDatas(beamFactorID.toInt());
    }else {
        qDebug() << "mCyntec not init";
    }
}

void DlgAIP::getCyntecBeamTableDatas(QString beamTableID)
{
    if(mCyntec){
        mCyntec->getBeamTableDatas(beamTableID.toInt());
    }else{
        qDebug() << "mCyntec not init";
    }
}

void DlgAIP::loadcfg()
{
    m_cfg->beginGroup("AIP");
    m_oldsavepath = m_cfg->value("selrefpath", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    m_cfg->endGroup();
}

void DlgAIP::savecfg()
{
    m_cfg->beginGroup("AIP");
    m_cfg->setValue("selrefpath", m_oldsavepath);
    m_cfg->endGroup();
}
