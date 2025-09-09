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
    connect(ui->pbSelReffile, &QPushButton::clicked, this, &DlgHanwha::onSelReffileClicked);
    //Hanwha
    connect(ui->HanwhaBeamDirectionID, &QComboBox::currentTextChanged, this, &DlgHanwha::onHanwhaBeamDirectionIDChanged);
    connect(ui->HanwhaBeamType, &QComboBox::currentTextChanged, this, &DlgHanwha::onHanwhaBeamTypeTextChanged);
    connect(ui->pbHanwhaBeamTable, &QPushButton::clicked, this, &DlgHanwha::onHanwhaBeamTableClicked);
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

void DlgHanwha::onHanwhaBeamDirectionIDChanged(QString newID)
{
    if (!newID.isEmpty()){
        if (mHanwha){
            // qDebug() << "onHanwhaBeamTableIDChanged:" << newID;
            mHanwha->getBeamTableData(newID.toInt());
        }
    }
}

void DlgHanwha::onHanwhaBeamTypeTextChanged(QString newBeamType)
{
    if (mHanwhaBeamTypeGroup.contains(newBeamType)){
        QStringList data= mHanwhaBeamTypeGroup.value(newBeamType);
        ui->HanwhaBeamDirectionID->clear();
        ui->HanwhaBeamDirectionID->insertItems(0, data);
    }else{
        // qDebug() << "No '" <<newBeamType<< "' in mHanwhaBeamTypeGroup";
    }
}

void DlgHanwha::onHanwhaBeamTableClicked(bool checked)
{
    Q_UNUSED(checked)
    if (mHanwha){
        FrmBeamTable *hBeamT = new FrmBeamTable(AIP::ModuleType::Hanwha);
        connect(this, &DlgHanwha::accepted, hBeamT, &FrmBeamTable::close);
        connect(this, &DlgHanwha::rejected, hBeamT, &FrmBeamTable::close);
        // connect(this, &DlgHanwha::finished, this, &DlgHanwha::onCloseHanwhaBeamTable);
        connect(this, &DlgHanwha::closeall, hBeamT, &FrmBeamTable::close);
        // connect(ui->HanwhaBeamDirectionID, &QComboBox::currentTextChanged,
        //         hBeamT, &FrmBeamTable::onSelectEllipse);
        //
        QString beamtype = ui->HanwhaBeamType->currentText();
        hBeamT->setWindowTitle(hBeamT->windowTitle()+"-"+beamtype);
        hBeamT->setGridPoints(mHanwha->getBeamTableDatas(beamtype));
        hBeamT->show();
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

void DlgHanwha::onUpdateBeamTypeGroup(QMap<QString, QStringList> data)
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
void DlgHanwha::loadcfg()
{
    m_cfg->beginGroup("AIP");
    m_oldsavepath = m_cfg->value("selrefpath", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    m_cfg->endGroup();

    // ui->CyntecElementMap->setCurrentText(0);
    // onHanwhaBeamTypeTextChanged(ui->HanwhaBeamType->currentText());
}

void DlgHanwha::savecfg()
{
    m_cfg->beginGroup("AIP");
    m_cfg->setValue("selrefpath", m_oldsavepath);
    m_cfg->endGroup();
}
