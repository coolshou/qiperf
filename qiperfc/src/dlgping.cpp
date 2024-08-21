#include "dlgping.h"
#include "ui_dlgping.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
#include <QMessageBox>
#include <QCloseEvent>

DlgPing::DlgPing(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgPing)
{
    ui->setupUi(this);
//    connect(this, &QDialog::finished, this, &DlgPing::onAccepted);
//    connect(this, &QDialog::accepted, this, &DlgPing::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DlgPing::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DlgPing::onRejected);
}

DlgPing::~DlgPing()
{
    delete ui;
}

QString DlgPing::getJsonstr()
{
    QJsonDocument doc = QJsonDocument();
    QJsonObject objRoot = doc.object();
    objRoot.insert("managerIP", ui->cb_managerIP->currentText());
    objRoot.insert("target", ui->cb_targetIP->currentText());
    objRoot.insert("count", ui->sb_Count->value());
    objRoot.insert("timeout", ui->sb_Timeout->value());
    objRoot.insert("interval", ui->sb_Interval->value());
    objRoot.insert("packetsize", ui->sb_PacketSize->value());
    objRoot.insert("source", ui->cb_bindIP->currentText());
    objRoot.insert("ttl", ui->sb_TTL->value());
    doc.setObject(objRoot);
    return doc.toJson(QJsonDocument::Compact);
}

void DlgPing::onAccepted()
{
    if (isRequireConfigMet()){
        // Condition met, accept the dialog
        accept();  // This closes the dialog with QDialog::Accepted
    }else {
//        QDialog::reject();
        return;
    }
}

void DlgPing::onRejected()
{
    // You can add condition checks here as well, or just reject
    reject();  // Close the dialog on Cancel
}

void DlgPing::changeEvent(QEvent *e)
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

bool DlgPing::isRequireConfigMet()
{
    //check require fields value
    QHostAddress address;
    if (!address.setAddress(ui->cb_managerIP->currentText())){
        QMessageBox::warning(this, tr("WARNING!!"),
                             tr("Please specify manager PC's ip address!!"),
                             QMessageBox::Ok);
        ui->cb_managerIP->setFocus();
        return false;
    }
    if (!address.setAddress(ui->cb_targetIP->currentText())){
        QMessageBox::warning(this, tr("WARNING!!"),
                             tr("Please specify ping target ip address!!"),
                             QMessageBox::Ok);
        ui->cb_targetIP->setFocus();
        return false;
    }
    return true;
}
