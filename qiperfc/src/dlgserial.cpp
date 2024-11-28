#include "dlgserial.h"
#include "ui_dlgserial.h"

#include <QDebug>

DlgSerial::DlgSerial(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgSerial)
{
    ui->setupUi(this);
    connect(ui->cbManager, &QComboBox::currentTextChanged, this , &DlgSerial::onChangeSerial);
}

DlgSerial::~DlgSerial()
{
    delete ui;
}

void DlgSerial::setSerialData(QMap<QString, QStringList> data)
{
    m_serials = data;
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
    qDebug() << "onChangeSerial:" << m_serials[text];
    QStringList ss = m_serials[text];
    if (!ss.isEmpty()){
        ui->portNameBox->addItems(ss);
    }

}
