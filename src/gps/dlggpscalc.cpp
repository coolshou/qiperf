#include "dlggpscalc.h"
#include "ui_dlggpscalc.h"

#include "gpsfunc.h"

#include <QTableWidgetItem>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <QDebug>


DlgGpsCalc::DlgGpsCalc(QSettings *cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgGpsCalc), m_cfg(cfg)
{
    ui->setupUi(this);
    ui->pbTaipei101SkyTree->setVisible(false);
    initAction();
    m_dlgOSM = new DlgOpenStreetMap();
    connect(ui->pbTaipei101SkyTree, &QPushButton::clicked, this, &DlgGpsCalc::onTaipei101SkyTree);
    connect(ui->pbCalc, &QPushButton::clicked, this, &DlgGpsCalc::onCalcCliecked);
    connect(ui->pbShowMap, &QPushButton::clicked, this, &DlgGpsCalc::onShowMap);
    connect(ui->pbClear, &QPushButton::clicked, m_clearAction, &QAction::triggered);
    connect(ui->pbToDMS, &QPushButton::clicked, this, &DlgGpsCalc::onToDMS);
    connect(ui->pbToDegree, &QPushButton::clicked, this, &DlgGpsCalc::onToDegree);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested,
            this, &DlgGpsCalc::showContextMenu);
    connect(m_dlgOSM, &DlgOpenStreetMap::loadFinished, this, &DlgGpsCalc::onLoadFinished);
    connect(this, &DlgGpsCalc::TileAvailable, this , &DlgGpsCalc::onTileAvailable);
    isTileAvailable();
}

DlgGpsCalc::~DlgGpsCalc()
{
    delete ui;
}

void DlgGpsCalc::isTileAvailable()
{
    // check if OpenStreetMapTile can be access
    // QString tile = getTile();
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    // QUrl url(tile);
    // QString ntile = tile.replace(url.path(), "");
    // qDebug() << "isTileAvailable: ntile: " << ntile;
    //TODO: how to check tile.openstreetmap.org is accessable?
    QString ntile = "https://www.openstreetmap.org";
    QUrl nurl(ntile);
    QNetworkRequest request(nurl);
    reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, &DlgGpsCalc::onCheckTileFinished);

}

QString DlgGpsCalc::getTile()
{
    m_cfg->beginGroup("gps");
    QString tile = m_cfg->value("OpenStreetMapTile").toString();
    m_cfg->endGroup();
    return tile;
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

void DlgGpsCalc::initAction()
{
    m_contextMenu = new QMenu(this);

    m_insertAction = m_contextMenu->addAction("Insert");
    connect(m_insertAction, &QAction::triggered, this , &DlgGpsCalc::onInsert);
    m_deleteAction = m_contextMenu->addAction("Delete");
    connect(m_deleteAction, &QAction::triggered, this , &DlgGpsCalc::onDelete);
    m_clearAction = new QAction("clear");
        // m_contextMenu->addAction("clear");
    connect(m_clearAction, &QAction::triggered, this , &DlgGpsCalc::onClear);
}

void DlgGpsCalc::onInsert(bool checked)
{
    Q_UNUSED(checked)
    int iRow = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(iRow+1);
    ui->tableWidget->setItem(iRow, 0, new QTableWidgetItem("Pos" + QString::number(iRow)));

}

void DlgGpsCalc::onDelete(bool checked)
{
    Q_UNUSED(checked)
    int iRow = ui->tableWidget->currentRow();//->selectRow();
    qDebug() << "onDelete:" <<  QString::number(iRow);
     ui->tableWidget->removeRow(iRow);
}

void DlgGpsCalc::onClear(bool checked)
{
    Q_UNUSED(checked)
    //TODO: ask before clear
    if (ui->tableWidget->rowCount()>0){
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);
    }
}

void DlgGpsCalc::onTaipei101SkyTree(bool checked)
{
    Q_UNUSED(checked)
    // 台北 101 大樓座標
    double lat1 = 25.033964;
    double lon1 = 121.564472;
    // ui->lat1->setText(QString::number(lat1));
    // ui->lon1->setText(QString::number(lon1));
    // 東京晴空塔座標
    double lat2 = 35.710046;
    double lon2 = 139.810718;
    // ui->lat2->setText(QString::number(lat2));
    // ui->lon2->setText(QString::number(lon2));

    ui->tableWidget->clearContents();
    // int irow = ui->tableWidget->currentRow();
    // ui->tableWidget->insertRow(irow+1);
    ui->tableWidget->setRowCount(2);

    ui->tableWidget->setItem(0,0, new QTableWidgetItem("taipei101"));
    ui->tableWidget->setItem(0,1, new QTableWidgetItem(QString::number(lat1)));
    ui->tableWidget->setItem(0,2, new QTableWidgetItem(QString::number(lon1)));
    ui->tableWidget->setItem(1,0, new QTableWidgetItem("SkyTree"));
    ui->tableWidget->setItem(1,1, new QTableWidgetItem(QString::number(lat2)));
    ui->tableWidget->setItem(1,2, new QTableWidgetItem(QString::number(lon2)));

}
void DlgGpsCalc::onCalcCliecked(bool checked)
{
    Q_UNUSED(checked)
    int iRow = ui->tableWidget->rowCount();
    if (iRow<2){
        QMessageBox::warning(this, tr("WARNING!!"),
                             tr("Please add at last two GPS location record"),
                             QMessageBox::Ok);
        return;
    }
    //check all cell have value
    QTableWidgetItem *itm=nullptr;
    int iCol = ui->tableWidget->columnCount();
    for (int row=0;row<iRow;row++){
        for (int col=0;col<iCol;col++){
            itm = ui->tableWidget->item(row,col);
            if(itm){
                if (itm->text().isEmpty()){
                    ui->tableWidget->setFocus();
                    ui->tableWidget->setCurrentCell(row,col);
                    return;
                }
            }
        }
    }
    QString pos1 = ui->tableWidget->item(0,0)->text();
    double lat1 = ui->tableWidget->item(0,1)->text().toDouble();
    double lon1 = ui->tableWidget->item(0,2)->text().toDouble();
    QString pos = "";
    double lat=0.0;
    double lon=0.0;
    double distance = 0;
    double azimuth = 0;
    double azimuth2 = 0;
    ui->twResult->setRowCount(iRow-1);

    for (int i=1; i<ui->tableWidget->rowCount(); i++){
        pos = ui->tableWidget->item(i,0)->text();
        lat = ui->tableWidget->item(i,1)->text().toDouble();
        lon = ui->tableWidget->item(i,2)->text().toDouble();
        if (ui->rbVincenty->isChecked()){
            VincentyResult vrs = vincentyInverse(lat1 , lon1, lat, lon);
            distance = vrs.distance/1000; // m -> KM
            azimuth = vrs.initialBearing;
            azimuth2 = vrs.finalBearing;
        }
        if (ui->rbHaversine->isChecked()){
            distance = haversine(lat1 , lon1, lat, lon);
            azimuth = calcBearing(lat1 , lon1, lat, lon);
            azimuth2 = calcBearing(lat, lon, lat1 , lon1);
        }
        ui->twResult->setItem(i-1, 0, new QTableWidgetItem(pos1 + " : " + pos));
        ui->twResult->setItem(i-1, 1, new QTableWidgetItem(QString::number(distance)));
        ui->twResult->setItem(i-1, 2, new QTableWidgetItem(QString::number(azimuth)));
        ui->twResult->setItem(i-1, 3, new QTableWidgetItem(QString::number(azimuth2)));
    }


}

void DlgGpsCalc::onShowMap(bool checked)
{
    Q_UNUSED(checked)
    QString errmsg ="";
    //TODO: check openstreetmap can be accessable
    QString tile = getTile();
    if (tile.isEmpty()){
        errmsg = "Require OpenStreetMapTile set eq: https://tile.openstreetmap.org/{z}/{x}/{y}.png";
        qDebug() << errmsg;
        QMessageBox::warning(this, "ERROR", errmsg, QMessageBox::Ok);
        return;
    }

    if (ui->tableWidget->rowCount()<1){
        QMessageBox::information(this, "Info",
                                 "Require at last one GPS locaton",
                                 QMessageBox::Ok);
        return;
    }

    if (m_dlgOSM){
        // QString pos1 = ui->tableWidget->item(0,0)->text();
        QString lat1 = ui->tableWidget->item(0,1)->text();
        QString lon1 = ui->tableWidget->item(0,2)->text();
        m_dlgOSM->load(tile , lat1, lon1);
        m_dlgOSM->show();
    }
}

void DlgGpsCalc::onToDMS(bool checked)
{
    Q_UNUSED(checked)
    //convert degree to DDD MM.MMMMS SS.SSSSS
    QString deg = ui->leDegree->text();
    bool ok;
    double degree = deg.toDouble(&ok);
    if (ok){
        DMS dms = degreeToDegreeMinSec(degree);
        ui->leDeg->setText(QString::number(dms.degrees));
        ui->leMin->setText(QString::number(dms.minutes));
        ui->leSec->setText(QString::number(dms.seconds));
    }

}

void DlgGpsCalc::onToDegree(bool checked)
{
    Q_UNUSED(checked)
    double degree = 0.0;
    //convert DDD MM.MMMMS SS.SSSSS to degree
    QString ddd = ui->leDeg->text();
    QString mm = ui->leMin->text();
    QString ss = ui->leSec->text();
    if ((!ddd.isEmpty())&&(!mm.isEmpty())&&(!ss.isEmpty())){
        degree = DegreeMinSecToDegree(ddd, mm, ss);
    }else if((!ddd.isEmpty())&&(!mm.isEmpty())){
        degree = DegreeMinToDegree(ddd, mm);
    }else{
        qDebug() << "unknown format" ;
    }
    if (degree>0){
        ui->leDegree->setText(QString::number(degree));
    }
}

void DlgGpsCalc::showContextMenu(const QPoint &pos)
{
    if (ui->tableWidget->rowCount()<1){
        m_deleteAction->setEnabled(false);
        m_deleteAction->setVisible(false);
    }else {
        m_deleteAction->setEnabled(true);
        m_deleteAction->setVisible(true);
    }
    QPoint globalPos = ui->tableWidget->mapToGlobal(pos);
    m_contextMenu->exec(globalPos);
    // m_contextMenu->show();
}

void DlgGpsCalc::onLoadFinished(bool ok)
{
    if (ok){
        if(m_dlgOSM){
            //reload marker
            m_dlgOSM->clearMarker();
            QString label;
            QString lat;
            QString lon;
            // marker
            for(int row=0;row<ui->tableWidget->rowCount();row++){
                label = ui->tableWidget->item(row,0)->text();
                lat = ui->tableWidget->item(row,1)->text();
                lon = ui->tableWidget->item(row,2)->text();
                m_dlgOSM->addMarker(lat, lon, label);
            }
        }
    }
}

void DlgGpsCalc::onTileAvailable(bool ok)
{
    ui->pbShowMap->setEnabled(ok);
}

void DlgGpsCalc::onCheckTileFinished()
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        qDebug() << "Response:" << response;
        emit TileAvailable(true);
    } else {
        QString errmsg= reply->errorString();
        qDebug() << "onCheckTileFinished: error: "  << errmsg;
        QMessageBox::warning(this, "Error", errmsg, QMessageBox::Ok);
        emit TileAvailable(false);
    }
}

void DlgGpsCalc::onCheckTileErrorOccurred(QNetworkReply::NetworkError errorcode)
{
    qDebug() << errorcode << " onCheckTileErrorOccurred: " << reply->errorString();
}
