#include "dlgopenstreetmap.h"
#include "ui_dlgopenstreetmap.h"

#include <QFile>

DlgOpenStreetMap::DlgOpenStreetMap(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgOpenStreetMap)
{
    ui->setupUi(this);
    markerCount = 0;
    connect(ui->pbSet, &QPushButton::clicked, this, &DlgOpenStreetMap::onSet);
    connect(ui->pbAddMarker, &QPushButton::clicked, this, &DlgOpenStreetMap::onAddMarker);
    connect(ui->pbClearMarker, &QPushButton::clicked, this, &DlgOpenStreetMap::onClearMarker);

    // pbSet
    view = new QWebEngineView(ui->wMap);
    connect(view,&QWebEngineView::loadFinished, this, &DlgOpenStreetMap::onLoadFinished);
    ui->vlMap->addWidget(view);
}

DlgOpenStreetMap::~DlgOpenStreetMap()
{
    delete ui;
}

void DlgOpenStreetMap::addMarker(QString lat, QString lon, QString label, QString marker)
{
    //    alert('markerid:' + ${markerid});
    QString js=QString("markerid = Object.keys(markerMap).length;\
    setMarker(markerid, %1, %2, '%3', '%4');").arg(lat, lon, label.replace("'", "\\'"), marker);
    // qDebug() << "addMarker: " << js;
    view->page()->runJavaScript(js);
}

void DlgOpenStreetMap::addDistLine(QString lat1, QString lon1, QString lat2, QString lon2, QString label)
{
    // add a distance line between two point with a label
    QString js=QString("addDistLine(%1, %2, %3, %4, '%5');").arg(lat1, lon1,
                                                                 lat2, lon2, label);
    qDebug() << "addDistLine: " << js;
    view->page()->runJavaScript(js);
}

void DlgOpenStreetMap::addAzimuthIndicator(QString lat, QString lon, QString azimuthDeg, QString lengthMeters)
{
    QString js=QString("drawAzimuthIndicator(%1, %2, %3, %4);").arg(lat, lon,
                                                                   azimuthDeg, lengthMeters);
    qDebug() << "addAzimuthIndicator: " << js;
    view->page()->runJavaScript(js);
}

void DlgOpenStreetMap::clearMarker()
{
    QString js=QString("clearMarker();");
    view->page()->runJavaScript(js);
}

void DlgOpenStreetMap::getMarkersCountAsync()
{
    view->page()->runJavaScript("Object.keys(markerMap).length;", [this](const QVariant &result) {
        markerCount = result.toInt();
        qDebug() << "Number of markers:" << markerCount;
        // Now use count here or call another method
    });

}

void DlgOpenStreetMap::load(QString tile, QString lat, QString lon)
{
    // Load HTML from resource using qrc path
    QFile file(":/openstreetmap/map.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString html = file.readAll();
        file.close();
        view->setHtml(html.arg(tile, lat,lon), QUrl("qrc:/"));  // Use qrc base URL for relative paths
        ui->leLat->setText(lat);
        ui->leLon->setText(lon);
    }
}

void DlgOpenStreetMap::onSet(bool checked)
{
    Q_UNUSED(checked)
    if (view){
        QString lat = ui->leLat->text();
        QString lon = ui->leLon->text();
        QString js=QString("map.setView([%1, %2], 16);").arg(lat, lon);
        view->page()->runJavaScript(js);
    }
}

void DlgOpenStreetMap::onAddMarker(bool checked)
{
    Q_UNUSED(checked)
    if (view){
        QString lat = ui->leLatMarker->text();
        QString lon = ui->leLonMarker->text();
        QString lable = ui->leLable->text();
        addMarker(lat, lon, lable);
        getMarkersCountAsync();
    }
}

void DlgOpenStreetMap::onClearMarker(bool checked)
{
    Q_UNUSED(checked)
    if (view){
        clearMarker();
    }
}

void DlgOpenStreetMap::onLoadFinished(bool ok)
{
    if (ok){
        emit loadFinished(ok);
    }
}
void DlgOpenStreetMap::changeEvent(QEvent *e)
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
