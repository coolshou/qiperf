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
    m_serials = data;
    ui->cbManager->clear();
    ui->cbManager->addItem("");
    ui->cbManager->addItems(m_serials.keys());
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
            ui->portNameBox->addItem(serialPortInfo.portName());
        }
    }

}

void DlgSerial::onPortConfig(bool checked)
{
    Q_UNUSED(checked)
    //TODO: portconfig
    portconfig->exec();
}
