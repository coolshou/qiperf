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
    // connect(view,&QWebEngineView::loadFinished, this, &DlgOpenStreetMap::onLoadFinished);
    ui->vlMap->addWidget(view);
}

DlgOpenStreetMap::~DlgOpenStreetMap()
{
    delete ui;
}

void DlgOpenStreetMap::addMarker(QString lat, QString lon, QString label)
{
    //    alert('markerid:' + ${markerid});
    QString js=QString("markerid = Object.keys(markerMap).length;\
    setMarker(markerid, %2, %3, '%4');").arg(lat, lon, label.replace("'", "\\'"));
    qDebug() << "addMarker: " << js;
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

void DlgOpenStreetMap::load(QString lat, QString lon)
{
    // Load HTML from resource using qrc path
    QFile file(":/openstreetmap/map.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString html = file.readAll();
        file.close();
        view->setHtml(html.arg(lat,lon), QUrl("qrc:/"));  // Use qrc base URL for relative paths
    }
}

void DlgOpenStreetMap::onSet(bool checked)
{
    Q_UNUSED(checked)
    if (view){
        QString lat = ui->leLat->text();
        QString lon = ui->leLon->text();
        QString js=QString("map.setView([%1, %2], 13);").arg(lat, lon);
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
        getMarkersCountAsync();
        addMarker("24.804162", "121.027736", "AM7");
        // QThread::sleep(2);//this will block ui
        getMarkersCountAsync();
        addMarker("24.802636", "121.022431", "CM7-1");
        // QThread::sleep(2);
        getMarkersCountAsync();
        addMarker("24.799918", "121.026686", "CM7-2");
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
