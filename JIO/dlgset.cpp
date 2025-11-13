#include "dlgset.h"
#include "ui_dlgset.h"

DlgSet::DlgSet(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgSet)
{
    ui->setupUi(this);
    connect(this, &DlgSet::accepted, this, &DlgSet::onAccepted);
}

DlgSet::~DlgSet()
{
    delete ui;
}

void DlgSet::setSSH(QString username, QString password)
{
    ui->leSSHUsername->setText(username);
    ui->leSSHPassword->setText(password);
}

void DlgSet::setWeb(QString username, QString password)
{
    ui->leWebUsername->setText(username);
    ui->leWebPassword->setText(password);

}

void DlgSet::setDuration(int duration)
{
    ui->sbDuration->setValue(duration);
}

void DlgSet::setCalc(QString distance, QString group, int kmeansfactor)
{
    if (distance.contains("Vincenty")){
        ui->rbVincenty->setChecked(true);
    }else{
        ui->rbHaversine->setChecked(true);
    }
    if (group.contains("Kmeans")){
        ui->rbKmeans->setChecked(true);
    }else if (group.contains("DBSCAN")){
        ui->rbDBSCAN->setChecked(true);
    }else{
        ui->rbAvg->setChecked(true);
    }
    ui->sbKmeansKFactor->setValue(kmeansfactor);
}

void DlgSet::changeEvent(QEvent *e)
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

void DlgSet::onAccepted()
{
    ControlBy ctl = ControlBy::None;
    //TODO: check text before send

    if (ui->cbControlBySSH->isChecked()){
        ctl = ControlBy::SSH;
    }
    if (ui->cbControlByQIperfd->isChecked()){
        ctl = ControlBy::QIPERFD;
    }
    emit updateSetting(ui->leSSHUsername->text(), ui->leSSHPassword->text(),
                       ui->leWebUsername->text(), ui->leWebPassword->text(),
                       ctl, ui->sbDuration->value());
    QString calcDistance;
    if (ui->rbVincenty->isChecked()){
        calcDistance = "Vincenty";
    } else {
        calcDistance = "Haversine";
    }
    QString calcGroup;
    if (ui->rbKmeans->isChecked()){
        calcGroup = "Kmeans";
    }else if(ui->rbDBSCAN->isChecked()){
        calcGroup = "DBSCAN";
    }else{
        calcGroup = "Avg";
    }
    emit updateCalc(calcDistance, calcGroup, ui->sbKmeansKFactor->value());
}

