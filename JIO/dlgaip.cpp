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


    mModuleType = AIP::ModuleType::Unknown;
    connect(ui->pbSelModule, &QPushButton::clicked, this, &DlgAIP::onSelModuleClicked);
    connect(ui->pbSelReffile, &QPushButton::clicked, this, &DlgAIP::onSelReffileClicked);
    connect(ui->cbAIPModule, &QComboBox::currentTextChanged, this, &DlgAIP::onChangeModule);
    connect(ui->sbX, &QDoubleSpinBox::valueChanged, this, &DlgAIP::onXValueChanged);
    connect(ui->sbY, &QDoubleSpinBox::valueChanged, this, &DlgAIP::onYValueChanged);
    connect(ui->sbZ, &QDoubleSpinBox::valueChanged, this, &DlgAIP::onZValueChanged);
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

void DlgAIP::onSelModuleClicked(bool checked)
{
    Q_UNUSED(checked)
    // show module's detail info cyntec/hanwha

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
        // QFile Hanwhafile(":/AIP/Hanwha.xlsx");
        // if (Hanwhafile.open(QIODevice::ReadOnly)) {
        //     qDebug() << "Calling Hanwha initBeamData with QFile...";
        //     mHanwha->initBeamData(&Hanwhafile);// Pass the address of the QFile object
        //     Hanwhafile.close(); // Close the file after initBeamData is done
        // } else {
        //     qDebug() << "Failed to open" << filename << "for reading:" << Hanwhafile.errorString();
        // }
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
                // mCyntec->initBeamData(newtext);

            }else if(mModuleType == AIP::ModuleType::Hanwha){
                qDebug() << "onRefFileTextChanged: read Hanwha xls file";
                // mHanwha->initBeamData(newtext);
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

void DlgAIP::loadcfg()
{
    m_cfg->beginGroup("AIP");
    m_oldsavepath = m_cfg->value("selrefpath", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    m_cfg->endGroup();

    // ui->CyntecElementMap->setCurrentText(0);
    // onHanwhaBeamTypeTextChanged(ui->HanwhaBeamType->currentText());
}

void DlgAIP::savecfg()
{
    m_cfg->beginGroup("AIP");
    m_cfg->setValue("selrefpath", m_oldsavepath);
    m_cfg->endGroup();
}
