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

#include "dlgaas.h"

#include <QDebug>

DlgAIP::DlgAIP(QSettings *cfg, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgAIP), m_cfg(cfg)
{
    mPosOffset=QVector3D(0.0, 0.0, 10.0);
    mAZOffset=0;
    ui->setupUi(this);
    connect(this, &DlgAIP::accepted, this, &DlgAIP::onAccepted);
    mModuleType = AIP::ModuleType::Unknown;
    connect(ui->pbSelModule, &QPushButton::clicked, this, &DlgAIP::onSelModuleClicked);
    connect(ui->cbAIPModule, &QComboBox::currentTextChanged, this, &DlgAIP::onChangeModule);
    connect(ui->sbX, &QDoubleSpinBox::valueChanged, this, &DlgAIP::onXValueChanged);
    connect(ui->sbY, &QDoubleSpinBox::valueChanged, this, &DlgAIP::onYValueChanged);
    connect(ui->sbZ, &QDoubleSpinBox::valueChanged, this, &DlgAIP::onZValueChanged);
    connect(ui->sbAZ, &QSpinBox::valueChanged, this,  &DlgAIP::onAZValueChanged);
    connect(ui->cbPreSetPos, &QComboBox::currentTextChanged, this, &DlgAIP::onPreSetPosTextChanged);
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
    // QString soffset = data.value("offset").toString();
    double x= data.value("offsetX").toDouble();
    double y= data.value("offsetY").toDouble();
    double z= data.value("offsetZ").toDouble();
    int   az= data.value("offsetAz").toInt();

    ui->sbX->setValue(x);
    ui->sbY->setValue(y);
    ui->sbZ->setValue(z);
    ui->sbAZ->setValue(az);
}

QJsonObject DlgAIP::getData()
{
    // get UI's value and turn into JSON format
    QJsonObject jobj;
    jobj["moduletype"] = static_cast<int>(mModuleType);
    jobj["offsetX"] = std::round(static_cast<double>(mPosOffset.x()) * 100.0) / 100.0;
    jobj["offsetY"] = std::round(static_cast<double>(mPosOffset.y()) * 100.0) / 100.0;
    jobj["offsetZ"] = std::round(static_cast<double>(mPosOffset.z()) * 100.0) / 100.0;;
    jobj["offsetAz"] = mAZOffset;
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
    if (mRow==0){
        if (mCol==DlgAAS::GPScols::AIP1){
            ui->cbPreSetPos->setCurrentIndex(2);
        }else{
            ui->cbPreSetPos->setCurrentIndex(3);
        }
    }else{
        ui->cbPreSetPos->setCurrentIndex(1);
    }
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
    // qDebug() << "DlgAIP::closeEvent:" << event;
    savecfg();
}

void DlgAIP::onAccepted()
{
    QJsonObject obj = getData();
    // QJsonDocument doc(obj);
    // QString strJson(doc.toJson(QJsonDocument::Compact));
    emit updateData(mRow, mCol, obj);
}

void DlgAIP::onSelModuleClicked(bool checked)
{
    Q_UNUSED(checked)
    // show module's detail info cyntec/hanwha

}

void DlgAIP::onChangeModule(QString newtext)
{
    ui->twBeam->clear();
    ui->twATT->clear();
    // qDebug() << "onChangeModule:" << newtext;
    if (newtext.startsWith("Cyntec")){
        mModuleType = AIP::ModuleType::Cyntec;
        QStringList hb;
        hb << "Name" << "TxA" << "TxB" << "RxA" << "RxB" ;
        ui->twBeam->setColumnCount(hb.count());
        ui->twBeam->setHorizontalHeaderLabels(hb);
        ui->twBeam->setRowCount(2);
        ui->twBeam->setItem(0,0, new QTableWidgetItem("BeamDirection"));
        ui->twBeam->setItem(1,0, new QTableWidgetItem("BeamFactor"));
        //

        QStringList hs;
        hs << "Name" << "A" << "B";
        ui->twATT->setColumnCount(hs.count());
        ui->twATT->setHorizontalHeaderLabels(hs);
        ui->twATT->setRowCount(3);
        ui->twATT->setItem(0,0, new QTableWidgetItem("TxAtt"));
        ui->twATT->setItem(1,0, new QTableWidgetItem("RxAtt"));
        ui->twATT->setItem(2,0, new QTableWidgetItem("RxIP3Att"));
    }else if (newtext.startsWith("Hanwha")){
        mModuleType = AIP::ModuleType::Hanwha;
    }else{
        mModuleType = AIP::ModuleType::Unknown;
    }
}

void DlgAIP::onPreSetPosTextChanged(QString newtext)
{
    double x = 0;
    double y = 0;
    double z = 0;
    if (newtext.startsWith("Client AIP0")){

    }else if (newtext.startsWith("AP AIP0")){

    }else if (newtext.startsWith("AP AIP1")){

    }
    ui->sbX->setValue(x);
    ui->sbY->setValue(y);
    ui->sbZ->setValue(z);
}

void DlgAIP::onXValueChanged(double value)
{
    mPosOffset.setX(value);
}

void DlgAIP::onYValueChanged(double value)
{
    mPosOffset.setY(value);
}

void DlgAIP::onZValueChanged(double value)
{
    mPosOffset.setZ(value);
}

void DlgAIP::onAZValueChanged(int value)
{
    mAZOffset = value;
}

void DlgAIP::loadcfg()
{
    m_cfg->beginGroup("AIP");
    // m_oldsavepath = m_cfg->value("selrefpath", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    m_cfg->endGroup();

    // ui->CyntecElementMap->setCurrentText(0);
    // onHanwhaBeamTypeTextChanged(ui->HanwhaBeamType->currentText());
}

void DlgAIP::savecfg()
{
    m_cfg->beginGroup("AIP");
    // m_cfg->setValue("selrefpath", m_oldsavepath);
    m_cfg->endGroup();
}
