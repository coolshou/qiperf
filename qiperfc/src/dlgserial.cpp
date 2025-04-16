#include "dlgserial.h"
#include "ui_dlgserial.h"

#include <QPushButton>
#include <QSerialPortInfo>
#include <QDebug>

DlgSerial::DlgSerial(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgSerial)
{
    ui->setupUi(this);
    portconfig = new PortSetBox();
    connect(ui->cbManager, &QComboBox::currentTextChanged, this , &DlgSerial::onChangeSerial);
    // connect(ui->pbPortConfig, &QPushButton::clicked, this, &DlgSerial::onPortConfig);
}

DlgSerial::~DlgSerial()
{
    delete ui;
}

void DlgSerial::setSerialData(QMap<QString, QStringList> data)
{
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

void DlgSerial::changeEvent(QEvent *e)
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
        //local serial ports
        // ui->portNameBox->addItems(QSerialPortInfo::availablePorts());
        QList<QSerialPortInfo> serialPortInfoList = QSerialPortInfo::availablePorts();
        foreach(QSerialPortInfo serialPortInfo, serialPortInfoList) {
            if (serialPortInfo.hasProductIdentifier() &&
                serialPortInfo.hasVendorIdentifier()){
                QString com;
#if defined(Q_OS_LINUX)
                com="/dev/";
#endif
                com=com+serialPortInfo.portName();
                ui->portNameBox->addItem(com);
            }
        }
    }

}

void DlgSerial::onPortConfig(bool checked)
{
    Q_UNUSED(checked)
    //TODO: portconfig
    portconfig->exec();
}
