#include "dlgserial.h"
#include "ui_dlgserial.h"

#include <QPushButton>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QStandardPaths>
#include "comm.h"

#include <QDebug>

DlgSerial::DlgSerial(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgSerial)
{
    ui->setupUi(this);
    ui->lbRemotePort->setVisible(false);
    ui->sbRemotePort->setVisible(false);
    oldpath = "";
    portconfig = new PortSetBox();
    connect(ui->cbManager, &QComboBox::currentTextChanged, this , &DlgSerial::onChangeSerial);
    connect(ui->pbPortConfig, &QPushButton::clicked, this, &DlgSerial::onPortConfig);
    connect(ui->pbSelectLogFile, &QPushButton::clicked, this, &DlgSerial::onSelectLogFile);
    // connect(ui->cbAddTimeStemp, &QCheckBox::stateChanged, this, &DlgSerial::onTimeStempChanged);
    initLocalSerialPort();
}

DlgSerial::~DlgSerial()
{
    delete ui;
}

void DlgSerial::setSerialData(QMap<QString, QStringList> data)
{
    // TODO: when known serials update, update this?
    //update cbManager list
    m_serials = data;
    ui->cbManager->clear();
    ui->cbManager->addItem("");
    ui->cbManager->addItems(m_serials.keys());
}

QString DlgSerial::getManagerIP()
{
    if (ui->cbManager->currentText().isEmpty()){
        return "127.0.0.1";
    }else{
        return ui->cbManager->currentText();
    }
}

QString DlgSerial::getSerialPort()
{
    return ui->portNameBox->currentText();
}

QString DlgSerial::getSerialCfg()
{
    //return:  comport:BaudRate:DataBits:Parity:StopBits:FlowControl
    return QString("%1:%2").arg(ui->baudRateBox->currentText(),
                                portconfig->getCfg());
}

QString DlgSerial::getLogFilename()
{
    if (ui->gbLogtoFile->isChecked()){
        return ui->leLogFilename->text();
    }else{
        return "";
    }
}

QString DlgSerial::getLogTimeStempFormat()
{
    if (ui->gbAddTimeStemp->isChecked()){
        return ui->leTimeStemp->text();
    } else {
        return "";
    }
}

void DlgSerial::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DlgSerial::closeEvent(QCloseEvent *event)
{
    if(ui->gbLogtoFile->isChecked()){
        if (ui->leLogFilename->text().isEmpty()){
            ui->leLogFilename->setFocus();
            event->ignore();
            return;
        }
        if (ui->gbAddTimeStemp->isChecked()){
            if(ui->leTimeStemp->text().isEmpty()){
                ui->leTimeStemp->setFocus();
                event->ignore();
                return;
            }
            //TODO: check time stemp format
        }
    }
}

void DlgSerial::initLocalSerialPort()
{
    //list local serial port
    QList<QSerialPortInfo> serialPortInfoList = QSerialPortInfo::availablePorts();
    foreach(QSerialPortInfo serialPortInfo, serialPortInfoList) {
#if defined(Q_OS_LINUX)
        if (serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier())
        // qDebug() << "ignore " << serialPortInfo.portName();
        // continue;
        // }
#endif
        {
            QString com;
#if defined(Q_OS_LINUX)
            com="/dev/";
#endif
            com=com+serialPortInfo.portName();
            ui->portNameBox->addItem(com);
        }
    }
}

void DlgSerial::onChangeSerial(QString text)
{
    ui->portNameBox->clear();
    if (!text.isEmpty()){
        qDebug() << "onChangeSerial:" << m_serials[text];
        QStringList ss = m_serials[text];
        if (!ss.isEmpty()){
            ui->portNameBox->addItems(ss);
        }
    }else {
        initLocalSerialPort();
    }

}

void DlgSerial::onPortConfig(bool checked)
{
    Q_UNUSED(checked)
    //TODO: portconfig
    portconfig->exec();
}

void DlgSerial::onSelectLogFile(bool checked)
{
    Q_UNUSED(checked)
    //select log filename, can be not exist
    if (oldpath.isEmpty()){
        oldpath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString path = oldpath + QDir::separator() + "serial.log";
    QString fileName = QFileDialog::getSaveFileName(this, tr("set log filename"),
                                                    path, tr(ALL_EXT_FILTER));
    if (!fileName.isEmpty()){
        ui->leLogFilename->setText(fileName);
        QFileInfo fi(fileName);
        oldpath = fi.path();
    }
}

void DlgSerial::onTimeStempChanged(int checkstatus)
{
    //TODO: onTimeStempChanged
    if (checkstatus== Qt::CheckState::Checked){
        ui->lbTimeStemp->setEnabled(true);
        ui->leTimeStemp->setEnabled(true);
    }else{
        ui->lbTimeStemp->setEnabled(false);
        ui->leTimeStemp->setEnabled(false);
    }
}
