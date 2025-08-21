#include "dlgaip.h"
#include "ui_dlgaip.h"

#include <QFile>
#include <QStandardPaths>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QResource>

#include <QDebug>


#include "comm.h"

DlgAIP::DlgAIP(QSettings *cfg, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgAIP), m_cfg(cfg)
{
    mPosOffset=QVector3D(0.0, 0.0, 10.0);
    ui->setupUi(this);
    connect(this, &DlgAIP::accepted, this, &DlgAIP::onAccepted);
    //Cyntec
    connect(ui->CyntecBeamFactorID, &QComboBox::currentTextChanged, this, &DlgAIP::onCyntecBeamFactorIDChanged);
    connect(ui->CyntecBeamTableID, &QComboBox::currentTextChanged, this, &DlgAIP::onCyntecBeamTableIDChanged);
    connect(ui->CyntecElementMap, &QComboBox::currentTextChanged, this, &DlgAIP::onCyntecElementMapChanged);
    //Hanwha
    connect(ui->HanwhaBeamTableID, &QComboBox::currentTextChanged, this, &DlgAIP::onHanwhaBeamTableIDChanged);
    connect(ui->HanwhaBeamType, &QComboBox::currentTextChanged, this, &DlgAIP::onHanwhaBeamTypeTextChanged);
    mModuleType = AIP::ModuleType::Unknown;
    connect(ui->pbSelReffile, &QPushButton::clicked, this, &DlgAIP::onSelReffileClicked);
    connect(ui->cbAIPModule, &QComboBox::currentTextChanged, this, &DlgAIP::onChangeModule);
    connect(ui->sbX, &QDoubleSpinBox::valueChanged, this, &DlgAIP::onXValueChanged);
    connect(ui->sbY, &QDoubleSpinBox::valueChanged, this, &DlgAIP::onYValueChanged);
    connect(ui->sbZ, &QDoubleSpinBox::valueChanged, this, &DlgAIP::onZValueChanged);
    connect(ui->cbPreSetPos, &QComboBox::currentTextChanged, this, &DlgAIP::onPreSetPosTextChanged);
    mCyntec = new Cyntec();
    connect(mCyntec, &Cyntec::newBeamFactorIDs, this, &DlgAIP::onNewCyntecBeamFactorIDs);
    connect(mCyntec, &Cyntec::newBeamTableIDs, this, &DlgAIP::onNewCyntecBeamTableIDs);
    connect(mCyntec, &Cyntec::updateBeamFactorData, this, &DlgAIP::onUpdateCynteBeamFactorData);
    connect(mCyntec, &Cyntec::updateBeamTableData, this, &DlgAIP::onUpdateCyntecBeamTableData);
    mHanwha = new Hanwha();
    // connect(mHanwha, &Hanwha::newBeamTableIDs, this, &DlgAIP::onNewHanwhaBeamTableIDs);
    connect(mHanwha, &Hanwha::updateBeamTableData, this, &DlgAIP::onUpdateHanwhaBeamTableData);
    connect(mHanwha, &Hanwha::updateBeamTypes, this, &DlgAIP::onUpdateBeamTypes);
    connect(mHanwha, &Hanwha::updateBeamTypeGroup, this, &DlgAIP::onUpdateBeamTypeGroup);
    connect(ui->pbCyntecBeamTable, &QPushButton::clicked, this, &DlgAIP::onCyntecBeamTableClicked);
    connect(ui->pbHanwhaBeamTable, &QPushButton::clicked, this, &DlgAIP::onHanwhaBeamTableClicked);
    loadcfg();
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
    }else{
        qDebug() << "DlgAIP::loadData Error: " << error.errorString()
                 << " sdata:" << sdata;
    }
}

void DlgAIP::loadData(QJsonObject data)
{
    //load json data and show on UI
    int iModuletype = data.value("moduletype").toInt();
    // qDebug() << "loadData, iModuletype:" << QString::number(iModuletype);
    ui->cbAIPModule->setCurrentIndex(iModuletype);
    if (iModuletype>0){
        QString smodel="...";
        if (static_cast<AIP::ModuleType>(iModuletype)==AIP::ModuleType::Cyntec){
            smodel="C"; //Cyntec
        }else if (static_cast<AIP::ModuleType>(iModuletype)==AIP::ModuleType::Hanwha){
            smodel="H"; //Hanwha
        }
        emit updateModelType(mRow, mCol, smodel);
    }
    QString soffset = data.value("offset").toString();
    double x=0.0;
    double y=0.0;
    double z=0.0;
    if (!soffset.isEmpty()){
        QStringList ds = soffset.split(",");
        if (ds.length()==3){
            x = ds[0].toDouble();
            y = ds[1].toDouble();
            z = ds[2].toDouble();
        }
    }
    // qDebug() << "loadData, x,y,z=" << QString::number(x) << " , "
    //          << QString::number(y) << " , " << QString::number(z);
    ui->sbX->setValue(x);
    ui->sbY->setValue(y);
    ui->sbZ->setValue(z);
}

QJsonObject DlgAIP::getData()
{
    // get UI's value and turn into JSON format
    QJsonObject jobj;
    jobj["moduletype"] = static_cast<int>(mModuleType);
    jobj["offset"] = QString("%1,%2,%3").arg(QString::number(mPosOffset.x(), 'f' ,2),
                                             QString::number(mPosOffset.y(), 'f' ,2),
                                             QString::number(mPosOffset.z(), 'f' ,2));
    qDebug() << "get Json data" << jobj;
    return jobj;
}

void DlgAIP::setPosOffset(float xpos, float ypos, float zpos)
{
    qDebug() << "X:" << xpos << " Y:" << ypos << " Z:" << zpos;
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
    qDebug() << "DlgAIP::closeEvent:" << event;
    savecfg();
}

void DlgAIP::onAccepted()
{
    QJsonObject obj = getData();
    // QJsonDocument doc(obj);
    // QString strJson(doc.toJson(QJsonDocument::Compact));
    emit updateData(mRow, mCol, obj);
}

void DlgAIP::onSelReffileClicked(bool checked)
{
    Q_UNUSED(checked)
    if (ui->cbAIPModule->currentText().isEmpty()){
        QString msg = QString("AIP module must select first");
        QMessageBox::warning(this, tr("WARNING!!"),
                             msg,
                             QMessageBox::Ok);
        ui->cbAIPModule->setFocus();
        return;
    }
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
        ui->lbRefFile->setText(fileName);
        onRefFileTextChanged(fileName);
        QFileInfo fileInfo(fileName);
        m_oldsavepath = fileInfo.path();
    }
}

void DlgAIP::onChangeModule(QString newtext)
{
    // qDebug() << "onChangeModule:" << newtext;
    QString filename="";
    int idxCyntec =ui->tabWidget->indexOf(ui->tabCyntec);
    int idxHanwha =ui->tabWidget->indexOf(ui->tabHanwha);
    if (newtext.startsWith("Cyntec")){
        QResource resCyntec(":/AIP/Cyntec.xlsx");
        filename = resCyntec.fileName();
        mModuleType = AIP::ModuleType::Cyntec;
        QFile Cyntecfile(":/AIP/Cyntec.xlsx");
        if (Cyntecfile.open(QIODevice::ReadOnly)) {
            qDebug() << "Calling Cyntec initBeamData with QFile...";
            mCyntec->initBeamData(&Cyntecfile);// Pass the address of the QFile object
            Cyntecfile.close(); // Close the file after initBeamData is done
        } else {
            qDebug() << "Failed to open" << filename << "for reading:" << Cyntecfile.errorString();
        }
        ui->tabWidget->setTabVisible(idxCyntec, true);
        ui->tabWidget->setTabVisible(idxHanwha, false);
    }else if (newtext.startsWith("Hanwha")){
        QResource resHanwha(":/AIP/Hanwha.xlsx");
        filename = resHanwha.fileName();
        QFile Hanwhafile(":/AIP/Hanwha.xlsx");
        if (Hanwhafile.open(QIODevice::ReadOnly)) {
            qDebug() << "Calling Hanwha initBeamData with QFile...";
            mHanwha->initBeamData(&Hanwhafile);// Pass the address of the QFile object
            Hanwhafile.close(); // Close the file after initBeamData is done
        } else {
            qDebug() << "Failed to open" << filename << "for reading:" << Hanwhafile.errorString();
        }
        mModuleType = AIP::ModuleType::Hanwha;
        ui->tabWidget->setTabVisible(idxCyntec, false);
        ui->tabWidget->setTabVisible(idxHanwha, true);
    }else{
        mModuleType = AIP::ModuleType::Unknown;
        ui->tabWidget->setTabVisible(idxCyntec, false);
        ui->tabWidget->setTabVisible(idxHanwha, false);
    }
    ui->lbRefFile->setText(filename);
}

void DlgAIP::onRefFileTextChanged(QString newtext)
{
    if (!newtext.isEmpty()){
        QFile f(newtext);
        if (f.exists()){
            if(mModuleType == AIP::ModuleType::Cyntec){
                // read Cyntec's xls file
                mCyntec->initBeamData(newtext);

            }else if(mModuleType == AIP::ModuleType::Hanwha){
                qDebug() << "onRefFileTextChanged: read Hanwha xls file";
                mHanwha->initBeamData(newtext);
            }else {
                qDebug() << "onRefFileTextChanged: unknows module type";
            }
        }else {
            qDebug() << "File not exist: " << newtext;
            QString msg = QString("File not exist: %1").arg(newtext);
            QMessageBox::warning(this, tr("WARNING!!"),
                                 msg,
                                 QMessageBox::Ok);
        }
    }else{
        qDebug() << "No ref xlsx file";
    }

}

void DlgAIP::onPreSetPosTextChanged(QString newtext)
{
    double x = 0;
    double y = 0;
    double z = 0;
    if (newtext.startsWith("CM7 AIP0")){

    }else if (newtext.startsWith("AM7 AIP0")){

    }else if (newtext.startsWith("AM7 AIP1")){

    }
    ui->sbX->setValue(x);
    ui->sbY->setValue(y);
    ui->sbZ->setValue(z);
}

void DlgAIP::onXValueChanged(double value)
{
    // qDebug() << " X ValueChanged:" << value;
    mPosOffset.setX(value);
}

void DlgAIP::onYValueChanged(double value)
{
    // qDebug() << " Y ValueChanged:" << value;
    mPosOffset.setY(value);
}

void DlgAIP::onZValueChanged(double value)
{
    // qDebug() << " Z ValueChanged:" << value;
    mPosOffset.setZ(value);
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
    Q_UNUSED(newBeamTableID)
    if (!newBeamTableID.isEmpty()){
        if (mCyntec){
            mCyntec->getBeamTableData(newBeamTableID.toInt());
        }
    }
}

void DlgAIP::onHanwhaBeamTableIDChanged(QString newBeamTableID)
{
    Q_UNUSED(newBeamTableID)
    if (!newBeamTableID.isEmpty()){
        if (mHanwha){
            // qDebug() << "onHanwhaBeamTableIDChanged:" << newBeamTableID;
            mHanwha->getBeamTableData(newBeamTableID.toInt());
        }
    }
}

void DlgAIP::onHanwhaBeamTypeTextChanged(QString newBeamType)
{
    if (mHanwhaBeamTypeGroup.contains(newBeamType)){
        QStringList data= mHanwhaBeamTypeGroup.value(newBeamType);
        ui->HanwhaBeamTableID->clear();
        ui->HanwhaBeamTableID->insertItems(0, data);
    }else{
        qDebug() << "No '" <<newBeamType<< "' in mHanwhaBeamTypeGroup";
    }
}

void DlgAIP::onNewCyntecBeamFactorIDs(QStringList keys)
{
    // qDebug() << "onNewCyntecBeamFactorIDs:" << keys;
    ui->CyntecBeamFactorID->clear();
    ui->CyntecBeamFactorID->insertItems(0, keys);
}

void DlgAIP::onNewCyntecBeamTableIDs(QStringList keys)
{
    // qDebug() << "onNewCyntecBeamTableIDs:" << keys;
    ui->CyntecBeamTableID->clear();
    ui->CyntecBeamTableID->insertItems(0, keys);
}

void DlgAIP::onNewHanwhaBeamTableIDs(QStringList keys)
{
    ui->HanwhaBeamTableID->clear();
    ui->HanwhaBeamTableID->insertItems(0, keys);
}

void DlgAIP::onUpdateCynteBeamFactorData(QString elementMap, int attDb, double azBW, double elBW)
{
    int idx = ui->CyntecElementMap->findText(elementMap);
    if (idx != ui->CyntecElementMap->currentIndex()){
        ui->CyntecElementMap->setCurrentIndex(idx);
    }
    ui->CyntecATT->setValue(attDb);
    ui->CyntecAzimuthBW->setValue(azBW);
    ui->CyntecElevationBW->setValue(elBW);
}

void DlgAIP::onUpdateCyntecBeamTableData(double az, double el, double azBW, double elBW)
{
    Q_UNUSED(azBW)
    Q_UNUSED(elBW)
    ui->CyntecAZ->setValue(az);
    ui->CyntecEL->setValue(el);
}

void DlgAIP::onCyntecBeamTableClicked(bool checked)
{
    Q_UNUSED(checked)
    if (mCyntec){
        FrmBeamTable *cBeamT = new FrmBeamTable(AIP::ModuleType::Cyntec);
        connect(this, &DlgAIP::finished, cBeamT, &FrmBeamTable::close);
        connect(ui->CyntecBeamTableID, &QComboBox::currentTextChanged,
                cBeamT, &FrmBeamTable::selectEllipse);
        cBeamT->setGridPoints(mCyntec->getBeamTableDatas());
        cBeamT->show();
    }
}

void DlgAIP::onHanwhaBeamTableClicked(bool checked)
{
    Q_UNUSED(checked)
    if (mHanwha){
        FrmBeamTable *hBeamT = new FrmBeamTable(AIP::ModuleType::Hanwha);
        connect(this, &DlgAIP::accepted, hBeamT, &FrmBeamTable::close);
        connect(this, &DlgAIP::rejected, hBeamT, &FrmBeamTable::close);
        connect(this, &DlgAIP::finished, this, &DlgAIP::onCloseHanwhaBeamTable);
        connect(ui->HanwhaBeamTableID, &QComboBox::currentTextChanged,
                hBeamT, &FrmBeamTable::selectEllipse);
        //
        QString beamtype = ui->HanwhaBeamType->currentText();
        hBeamT->setWindowTitle(hBeamT->windowTitle()+"-"+beamtype);
        hBeamT->setGridPoints(mHanwha->getBeamTableDatas(beamtype));
        hBeamT->show();
    }
}

void DlgAIP::onUpdateHanwhaBeamTableData(double az, double el, double azBW, double elBW)
{
    Q_UNUSED(azBW)
    Q_UNUSED(elBW)
    ui->HanwhaAZ->setValue(az);
    ui->HanwhaEL->setValue(el);
}

void DlgAIP::onUpdateBeamTypes(QStringList beamtypes)
{
    ui->HanwhaBeamType->insertItems(0,beamtypes);
}

void DlgAIP::onUpdateBeamTypeGroup(QMap<QString, QStringList> data)
{
    mHanwhaBeamTypeGroup = data;
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
        mCyntec->getBeamTableData(beamTableID.toInt());
    }else{
        qDebug() << "mCyntec not init";
    }
}

void DlgAIP::onCloseHanwhaBeamTable(int code)
{
    qDebug() << "onCloseHanwhaBeamTable:" << code;
}

void DlgAIP::loadcfg()
{
    m_cfg->beginGroup("AIP");
    m_oldsavepath = m_cfg->value("selrefpath", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    m_cfg->endGroup();

    ui->CyntecElementMap->setCurrentText(0);
    onHanwhaBeamTypeTextChanged(ui->HanwhaBeamType->currentText());
}

void DlgAIP::savecfg()
{
    m_cfg->beginGroup("AIP");
    m_cfg->setValue("selrefpath", m_oldsavepath);
    m_cfg->endGroup();
}
