#include "dlghanwha.h"
#include "ui_dlghanwha.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QFileInfo>

#include "comm.h"

DlgHanwha::DlgHanwha(QSettings *cfg, Hanwha *hanwha, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgHanwha), m_cfg(cfg), mHanwha(hanwha)
{
    ui->setupUi(this);
    nBeamT = nullptr;
    wBeamT = nullptr;
    tBeamT = nullptr;
    qBeamT = nullptr;
    connect(ui->pbSelReffile, &QPushButton::clicked, this, &DlgHanwha::onSelReffileClicked);
    //Hanwha
    connect(ui->HanwhaBeamDirectionID, &QComboBox::currentTextChanged, this, &DlgHanwha::onHanwhaBeamDirectionIDChanged);
    connect(ui->HanwhaBeamType, &QComboBox::currentTextChanged, this, &DlgHanwha::onHanwhaBeamTypeTextChanged);
    connect(ui->pbHanwhaBeamTable, &QPushButton::clicked, this, &DlgHanwha::onHanwhaBeamTableClicked);
    connect(ui->pbAround, &QPushButton::clicked, this, &DlgHanwha::onSelectAroundID);
    loadcfg();
}

DlgHanwha::~DlgHanwha()
{
    delete ui;
}

QString DlgHanwha::getHanwhaBeamType()
{
    //get current HanwhaBeamType
    return ui->HanwhaBeamType->currentText();
}

FrmBeamTable *DlgHanwha::getBeamTable()
{
    QString beamtype = getHanwhaBeamType();
    if (beamtype.contains("NARROW")){
        return nBeamT;
    }else if (beamtype.contains("widebeam")){
        return wBeamT;
    }else if (beamtype.contains("TRI")){
        return tBeamT;
    }else{
        return qBeamT;
    }
}

void DlgHanwha::setBeamTable(FrmBeamTable *beamtable)
{
    QString beamtype = getHanwhaBeamType();
    if (beamtype.contains("NARROW")){
        nBeamT=beamtable;
    }else if (beamtype.contains("widebeam")){
        wBeamT=beamtable;
    }else if (beamtype.contains("TRI")){
        tBeamT=beamtable;
    }else{
        tBeamT=beamtable;
    }
}

void DlgHanwha::changeEvent(QEvent *e)
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

void DlgHanwha::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    savecfg();
    emit closeall();
}

void DlgHanwha::onSelReffileClicked(bool checked)
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

void DlgHanwha::onHanwhaBeamDirectionIDChanged(QString newBeamTableID)
{
    if (!newBeamTableID.isEmpty()){
        if (mHanwha){
            mHanwha->getBeamTableData(newBeamTableID.toInt());
        }
        emit SelectEllipse(newBeamTableID, true, Qt::red);
    }
}

void DlgHanwha::onHanwhaBeamTypeTextChanged(QString newBeamType)
{
    if (mHanwhaBeamTypeGroup.contains(newBeamType)){
        QList<int> dataList= mHanwhaBeamTypeGroup.value(newBeamType);
        QList<QString> stringList;
        for (int value : dataList) {
            stringList << QString::number(value);
        }
        ui->HanwhaBeamDirectionID->clear();
        ui->HanwhaBeamDirectionID->insertItems(0, stringList);
    }else{
        // qDebug() << "No '" <<newBeamType<< "' in mHanwhaBeamTypeGroup";
    }
}

void DlgHanwha::onHanwhaBeamTableClicked(bool checked)
{
    Q_UNUSED(checked)
    if (mHanwha){
        QString beamtype = getHanwhaBeamType();
        FrmBeamTable *beamtable;
        beamtable = getBeamTable();
        if (!beamtable){
            beamtable = new FrmBeamTable(AIP::ModuleType::Hanwha);
            connect(this, &DlgHanwha::accepted, beamtable, &FrmBeamTable::close);
            connect(this, &DlgHanwha::rejected, beamtable, &FrmBeamTable::close);
            // connect(this, &DlgHanwha::finished, this, &DlgHanwha::onCloseHanwhaBeamTable);
            connect(this, &DlgHanwha::closeall, beamtable, &FrmBeamTable::close);
            connect(this, &DlgHanwha::SelectEllipse, beamtable, &FrmBeamTable::onSelectEllipse);
            setBeamTable(beamtable);
        }
        if (!beamtable->windowTitle().contains(beamtype)){
            beamtable->setWindowTitle(beamtable->windowTitle()+"-"+beamtype);
        }
        beamtable->setGridPoints(mHanwha->getBeamTableDatas(beamtype));
        beamtable->activateWindow();
        beamtable->show();
    }
}

void DlgHanwha::onUpdateHanwhaBeamTableData(double az, double el, double azBW, double elBW)
{
    Q_UNUSED(azBW)
    Q_UNUSED(elBW)
    ui->HanwhaAZ->setValue(az);
    ui->HanwhaEL->setValue(el);
}

void DlgHanwha::onUpdateBeamTypes(QStringList beamtypes)
{
    ui->HanwhaBeamType->insertItems(0,beamtypes);
}

void DlgHanwha::onUpdateBeamTypeGroup(QMap<QString, QList<int>> data)
{
    mHanwhaBeamTypeGroup = data;
}

void DlgHanwha::onRefFileTextChanged(QString newtext)
{
    emit reffilechanged(newtext);
}

void DlgHanwha::setRefFileName(QString filename)
{
    ui->lbRefFile->setText(filename);
}

void DlgHanwha::onCloseHanwhaBeamTable(int code)
{
    qDebug() << "onCloseHanwhaBeamTable:" << code;
}
void DlgHanwha::onNewHanwhaBeamTableIDs(QStringList keys)
{
    ui->HanwhaBeamDirectionID->clear();
    ui->HanwhaBeamDirectionID->insertItems(0, keys);
}

void DlgHanwha::onSelectAroundID(bool checked)
{
    Q_UNUSED(checked)
    bool doClear=false;
    if (ui->cbClearMark->checkState()==Qt::CheckState::Checked){
        doClear = true;
    }

    int id = ui->HanwhaBeamDirectionID->currentText().toInt();
    QString beamtype = ui->HanwhaBeamType->currentText();
    int glimit = ui->sbAroundLimit->value();
    QVector<int> ds= mHanwha->findNearestNeighbors(id, beamtype, glimit);
    if (ds.length()>0){
        FrmBeamTable *beamtable;
        beamtable = getBeamTable();
        if (beamtable && doClear){
            beamtable->clearEllipseSelection();
        }
        foreach (int idx, ds){
            emit SelectEllipse(QString::number(idx), false, Qt::blue);
        }
    }
}
void DlgHanwha::loadcfg()
{
    m_cfg->beginGroup("AIP");
    m_oldsavepath = m_cfg->value("selrefpath", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    m_cfg->endGroup();

    // ui->CyntecElementMap->setCurrentText(0);
    // onHanwhaBeamTypeTextChanged(getHanwhaBeamType());
}

void DlgHanwha::savecfg()
{
    m_cfg->beginGroup("AIP");
    m_cfg->setValue("selrefpath", m_oldsavepath);
    m_cfg->endGroup();
}
