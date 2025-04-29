#include "dlggpscalc.h"
#include "ui_dlggpscalc.h"

#include "gpsfunc.h"

DlgGpsCalc::DlgGpsCalc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgGpsCalc)
{
    ui->setupUi(this);
    connect(ui->pbTaipei101SkyTree, &QPushButton::clicked, this, &DlgGpsCalc::onTaipei101SkyTree);
    connect(ui->pbCalc, &QPushButton::clicked, this, &DlgGpsCalc::onCalcCliecked);
    connect(ui->pbPos, &QPushButton::clicked, this, &DlgGpsCalc::onPosCliecked);
}

DlgGpsCalc::~DlgGpsCalc()
{
    delete ui;
}

void DlgGpsCalc::changeEvent(QEvent *e)
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

void DlgGpsCalc::onTaipei101SkyTree(bool checked)
{
    // 台北 101 大樓座標
    double lat1 = 25.033964;
    double lon1 = 121.564472;
    ui->lat1->setText(QString::number(lat1));
    ui->lon1->setText(QString::number(lon1));
    // 東京晴空塔座標
    double lat2 = 35.710046;
    double lon2 = 139.810718;
    ui->lat2->setText(QString::number(lat2));
    ui->lon2->setText(QString::number(lon2));

}
void DlgGpsCalc::onCalcCliecked(bool checked)
{
    Q_UNUSED(checked)
    if (ui->lat1->text().isEmpty()){
        return;
    }
    // TODO: check lat1/lon1/lat2/lon2 format
    // DD.mm.ss.sssss N/S/E/W
    // DD.mm.mmmmm N/S/E/W
    //
    double lat1 = ui->lat1->text().toDouble();
    double lon1 = ui->lon1->text().toDouble();
    double lat2 = ui->lat2->text().toDouble();
    double lon2 = ui->lon2->text().toDouble();

    double distance = 0;
    double azimuth = 0;
    double azimuth2 = 0;
    if (ui->rbVincenty->isChecked()){
        VincentyResult vrs = vincentyInverse(lat1 , lon1, lat2, lon2);
        distance = vrs.distance/1000; // m -> KM
        azimuth = vrs.initialBearing;
        azimuth2 = vrs.finalBearing;
    }
    if (ui->rbHaversine->isChecked()){
        azimuth = calcBearing(lat1 , lon1, lat2, lon2);
        azimuth2 = calcBearing(lat2, lon2,lat1 , lon1);
        distance = haversine(lat1 , lon1, lat2, lon2);
    }

    ui->azimuthGPS12->setText(QString::number(azimuth));
    ui->azimuthGPS21->setText(QString::number(azimuth2));
    ui->leDistance->setText(QString::number(distance));

}

void DlgGpsCalc::onPosCliecked(bool checked)
{
    Q_UNUSED(checked)
    // show google map
}
