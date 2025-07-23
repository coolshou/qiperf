#include "dlgjio.h"
#include "ui_dlgjio.h"

#include "../src/gps/gpsfunc.h"

#include <QTableWidgetItem>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QFileDialog>
#include <QStandardPaths>
#include <QPushButton>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>

#include "../src/gps/geotranslate.h"
#include "comm.h"
#include "../src/numberdelegate.h"

#include <QDebug>


DlgJIO::DlgJIO(QSettings *cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgJIO), m_cfg(cfg)
{
    m_debuglv=3;
    ui->setupUi(this);
    loadcfg();
    ui->pbShow3D->setVisible(false);
    ui->pbShowMap->setVisible(false);//html base map. not good to show correct position

    ui->tableWidget->setColumnWidth(GPScols::Latitude, 100);
    ui->tableWidget->setColumnWidth(GPScols::Longitude, 100);
    ui->tableWidget->setColumnWidth(GPScols::Altitude, 80);
    ui->tableWidget->setColumnWidth(GPScols::AIP1, 50);
    ui->tableWidget->setColumnWidth(GPScols::AIP2, 50);
    // Only accept Double
    NumberDelegate *dLatDelegate = new NumberDelegate(NumberDelegate::Double,
                                                      -90.0, 90.0, 6,
                                                      ui->tableWidget);
    ui->tableWidget->setItemDelegateForColumn(GPScols::Latitude, dLatDelegate);
    NumberDelegate *dLonDelegate = new NumberDelegate(NumberDelegate::Double,
                                                      -180.0, 180.0, 6,
                                                      ui->tableWidget);
    ui->tableWidget->setItemDelegateForColumn(GPScols::Longitude, dLonDelegate);
    NumberDelegate *dAltDelegate = new NumberDelegate(NumberDelegate::Double,
                                                      -10.0, 5500.0, 2,
                                                      ui->tableWidget);
    ui->tableWidget->setItemDelegateForColumn(GPScols::Altitude, dAltDelegate);

    ui->twResult->setColumnWidth(AZEIcols::Distance, 90);
    ui->twResult->setColumnWidth(AZEIcols::Azimuth1, 90);
    ui->twResult->setColumnWidth(AZEIcols::Azimuth2, 90);
    ui->twResult->setColumnWidth(AZEIcols::Elevation1, 100);
    ui->twResult->setColumnWidth(AZEIcols::Elevation2, 100);
    // Only accept Double
    NumberDelegate *dDelegate = new NumberDelegate(NumberDelegate::Double,
                                                   0.0, 10000.0, 2,
                                                   ui->tableWidget);
    ui->twResult->setItemDelegateForColumn(AZEIcols::Distance, dDelegate);
    NumberDelegate *dAziDelegate = new NumberDelegate(NumberDelegate::Double,
                                                   0.0, 360.0, 2,
                                                   ui->tableWidget);
    ui->twResult->setItemDelegateForColumn(AZEIcols::Azimuth1, dAziDelegate);
    ui->twResult->setItemDelegateForColumn(AZEIcols::Azimuth2, dAziDelegate);
    NumberDelegate *dElDelegate = new NumberDelegate(NumberDelegate::Double,
                                                   -90.0, 90.0, 2,
                                                   ui->tableWidget);
    ui->twResult->setItemDelegateForColumn(AZEIcols::Elevation1, dElDelegate);
    ui->twResult->setItemDelegateForColumn(AZEIcols::Elevation2, dElDelegate);


    initAction();
    m_dlgOSM = new DlgOpenStreetMap();
    connect(m_dlgOSM, &DlgOpenStreetMap::loadFinished, this, &DlgJIO::onLoadFinished);
    m_dlgGeo = new DlgGeoOSM();
    connect(m_dlgGeo, &DlgGeoOSM::loadFinished, this, &DlgJIO::onLoadFinished);

    connect(this, &DlgJIO::closeAll, m_dlgOSM, &DlgOpenStreetMap::close);
    connect(this, &DlgJIO::closeAll, m_dlgGeo, &DlgGeoOSM::close);
    connect(ui->pbLoad, &QPushButton::clicked, this, &DlgJIO::onLoadCliecked);
    connect(ui->pbSave, &QPushButton::clicked, this, &DlgJIO::onSaveCliecked);
    connect(ui->pbCalc, &QPushButton::clicked, this, &DlgJIO::onCalcCliecked);
    connect(ui->pbShowMap, &QPushButton::clicked, this, &DlgJIO::onShowMap);
    connect(ui->pbShowGeo, &QPushButton::clicked, this, &DlgJIO::onShowGeo);
    connect(ui->pbShow3D, &QPushButton::clicked, this, &DlgJIO::onShow3D);
    connect(ui->pbClear, &QPushButton::clicked, m_clearAction, &QAction::triggered);
    connect(ui->pbToDMS, &QPushButton::clicked, this, &DlgJIO::onToDMS);
    connect(ui->pbToDegree, &QPushButton::clicked, this, &DlgJIO::onToDegree);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested,
            this, &DlgJIO::showContextMenu);

    connect(this, &DlgJIO::TileAvailable, this , &DlgJIO::onTileAvailable);
    isTileAvailable();

    m_dlgaip = new DlgAIP(m_cfg, this);
    connect(m_dlgaip, &DlgAIP::updateData, this, &DlgJIO::onUpdateData);
    // connect(m_dlgaip, &DlgAIP::accepted, this, &DlgGpsCalc::onAcceptedAIP);
}

DlgJIO::~DlgJIO()
{
    delete ui;
}

void DlgJIO::isTileAvailable()
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

    connect(reply, &QNetworkReply::finished, this, &DlgJIO::onCheckTileFinished);

}

QString DlgJIO::getTile()
{
    m_cfg->beginGroup("gps");
    QString tile = m_cfg->value("OpenStreetMapTile").toString();
    m_cfg->endGroup();
    return tile;
}

void DlgJIO::setShowLine(bool show)
{
    showline = show;
}

void DlgJIO::clearData()
{
    //TODO: ask before clear
    if (ui->tableWidget->rowCount()>0){
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);
    }
    if (ui->twResult->rowCount()>0) {
        ui->twResult->clearContents();
        ui->twResult->setRowCount(0);
    }
}

void DlgJIO::changeEvent(QEvent *e)
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

void DlgJIO::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    savecfg();
    emit closeAll();
}

void DlgJIO::initAction()
{
    m_contextMenu = new QMenu(this);

    m_insertAction = m_contextMenu->addAction("Insert");
    connect(m_insertAction, &QAction::triggered, this , &DlgJIO::onInsert);
    m_deleteAction = m_contextMenu->addAction("Delete");
    connect(m_deleteAction, &QAction::triggered, this , &DlgJIO::onDelete);
    m_clearAction = new QAction("clear");
        // m_contextMenu->addAction("clear");
    connect(m_clearAction, &QAction::triggered, this , &DlgJIO::onClear);
}

void DlgJIO::onInsert(bool checked)
{
    Q_UNUSED(checked)
    onAddRow("New", 0.0, 0.0, 0.0);
}

void DlgJIO::onDelete(bool checked)
{
    Q_UNUSED(checked)
    int iRow = ui->tableWidget->currentRow();//->selectRow();
    qDebug() << "onDelete:" <<  QString::number(iRow);
    ui->tableWidget->removeRow(iRow);
}

void DlgJIO::onAddRow(QString name, double latitude, double longitude, double altitude)
{
    int iRow = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(iRow);
    QString err = QString("name: %1 ,latitude: %2 ,longitude: %3 ,altitude: %4").arg(name,
                           QString::number(latitude, 'f', 6),
                           QString::number(longitude, 'f', 6),
                           QString::number(altitude, 'f', 2));
    debug(err, 5);
    ui->tableWidget->setSortingEnabled(false);
    ui->tableWidget->setItem(iRow, GPScols::PositionName, new QTableWidgetItem(name));
    ui->tableWidget->setItem(iRow, GPScols::Latitude, new QTableWidgetItem(QString::number(latitude, 'f', 6)));
    ui->tableWidget->setItem(iRow, GPScols::Longitude, new QTableWidgetItem(QString::number(longitude, 'f', 6)));
    ui->tableWidget->setItem(iRow, GPScols::Altitude, new QTableWidgetItem(QString::number(altitude, 'f', 2)));
    QPushButton *btn1 = new QPushButton("...");
    int iCol = static_cast<int>(GPScols::AIP1);
    connect(btn1, &QPushButton::clicked, this, [this, iRow, iCol]() {
        handleButtonClicked(iRow, iCol);
    });
    ui->tableWidget->setCellWidget(iRow, GPScols::AIP1, btn1);
    if (iRow==0){
        QPushButton *btn2 = new QPushButton("...");
        iCol = static_cast<int>(GPScols::AIP2);
        connect(btn2, &QPushButton::clicked, this, [this, iRow, iCol]() {
            handleButtonClicked(iRow, iCol);
        });
        ui->tableWidget->setCellWidget(iRow, GPScols::AIP2, btn2);
    }else{
        // disable cell
        QTableWidgetItem *disableItem = new QTableWidgetItem("");
        disableItem->setFlags(disableItem->flags() & ~Qt::ItemIsEnabled); //disabled
        disableItem->setBackground(Qt::lightGray);
        disableItem->setFlags(disableItem->flags() & ~Qt::ItemIsSelectable); //not selectable
        ui->tableWidget->setItem(iRow, GPScols::AIP2, disableItem);
    }

    ui->tableWidget->setSortingEnabled(true);
}

void DlgJIO::onClear(bool checked)
{
    Q_UNUSED(checked)
    clearData();
}

void DlgJIO::onLoadCliecked(bool checked)
{
    Q_UNUSED(checked)
    QString path;
    if (!m_oldsavepath.isNull()){
        path = m_oldsavepath;
    }else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Load Position config from file"),
                                                    path ,
                                                    tr(QIPERF_EXT_FILTER_JSON));
    if (!fileName.isEmpty()){
        QFileInfo fileInfo(fileName);
        m_oldsavepath = fileInfo.path();
        onLoad(fileName);
    }
}

void DlgJIO::onSaveCliecked(bool checked)
{
    Q_UNUSED(checked)
    QString path;
    if (!m_oldsavepath.isNull()){
        path = m_oldsavepath;
    }else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Position config to file "),
                                                    path,
                                                    tr(QIPERF_EXT_FILTER_JSON));
    if (!fileName.isEmpty()){
        QFileInfo fi(fileName);
        QString ext = fi.suffix();
        if (ext.compare(QIPERF_EXT_JSON)!=0){
            fileName = fi.path()+ QDir::separator() + fi.baseName() + "."+ QIPERF_EXT_JSON;
        }
        if (onSave(fileName)){
            m_oldsavepath = fi.path();
        }
    }
}

void DlgJIO::onCalcCliecked(bool checked)
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
    // int iCol = ui->tableWidget->columnCount();
    int iCol = static_cast<int>(GPScols::AIP1);
    for (int row=0; row<iRow; row++){
        for (int col=0; col<iCol; col++){
            itm = ui->tableWidget->item(row, col);
            if(itm){
                if (itm->text().isEmpty()){
                    ui->tableWidget->setFocus();
                    ui->tableWidget->setCurrentCell(row, col);
                    qDebug() << "itm has no text (" << row << "," << col << ")";
                    return;
                }
            }
        }
    }
    QString pos1 = ui->tableWidget->item(0,GPScols::PositionName)->text();
    double lat1 = ui->tableWidget->item(0,GPScols::Latitude)->text().toDouble();
    double lon1 = ui->tableWidget->item(0,GPScols::Longitude)->text().toDouble();
    double altmsl1 = ui->tableWidget->item(0,GPScols::Altitude)->text().toDouble();
    // double msl1 = GeoTranslate::convertEllipsoidToMSL(lat1, lon1, alt1);
    // qDebug() << " Pos:" << pos1 << " Elevation hight:" << QString::number(msl1);
    QString pos = "";
    double lat=0.0;
    double lon=0.0;
    double altmsl=0.0;
    // double msl=0.0;
    double distance = 0;
    double azimuth = 0;
    double totalazimuth = 0.0;
    double azimuth2 = 0;
    double el1=0.0;
    double el2=0.0;
    double totalel=0.0;
    ui->twResult->setRowCount(iRow-1);

    for (int i=1; i<ui->tableWidget->rowCount(); i++){
        pos = ui->tableWidget->item(i,GPScols::PositionName)->text();
        lat = ui->tableWidget->item(i,GPScols::Latitude)->text().toDouble();
        lon = ui->tableWidget->item(i,GPScols::Longitude)->text().toDouble();
        altmsl = ui->tableWidget->item(i,GPScols::Altitude)->text().toDouble();
        // msl =  GeoTranslate::convertEllipsoidToMSL(lat, lon, alt);
        // qDebug() << " Pos:" << pos << " Elevation hight:" << QString::number(msl);
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
        ui->twResult->setItem(i-1, AZEIcols::Name, new QTableWidgetItem(pos1 + " : " + pos));
        ui->twResult->setItem(i-1, AZEIcols::Distance, new QTableWidgetItem(QString::number(distance)));
        ui->twResult->setItem(i-1, AZEIcols::Azimuth1, new QTableWidgetItem(QString::number(azimuth)));
        ui->twResult->setItem(i-1, AZEIcols::Azimuth2, new QTableWidgetItem(QString::number(azimuth2)));

        totalazimuth = totalazimuth + azimuth;
        el1 = GeoTranslate::calcElevationAngle(altmsl1, altmsl, distance*1000);
        el2 = GeoTranslate::calcElevationAngle(altmsl, altmsl1, distance*1000);
        qDebug() << " " << QString::number(altmsl1) << " - "  << QString::number(altmsl)
                 << " distance:" << QString::number(distance)
                 << " el1:" << QString::number(el1) << " el2:" << QString::number(el2);

        ui->twResult->setItem(i-1, AZEIcols::Elevation1, new QTableWidgetItem(QString::number(el1)));
        ui->twResult->setItem(i-1, AZEIcols::Elevation2, new QTableWidgetItem(QString::number(el2)));
        totalel = totalel + el1;
        if (showline){
            if (m_dlgOSM){
                m_dlgOSM->addDistLine(QString::number(lat1), QString::number(lon1),
                                      QString::number(lat), QString::number(lon),
                                      QString::number(distance));
            }
        }
    }
    double azimuthDegree = totalazimuth/ui->twResult->rowCount();
    ui->leExpectAzimuth->setText(QString::number(azimuthDegree));
    qDebug() << "totalel:" << QString::number(totalel);
    double elDegree = totalel/ui->twResult->rowCount();
    ui->leExpectElevation->setText(QString::number(elDegree));

    if (showline){
        if (m_dlgOSM){
            m_dlgOSM->addAzimuthIndicator(QString::number(lat1), QString::number(lon1),
                                          QString::number(azimuthDegree), QString::number(200));
        }
    }

}

void DlgJIO::onShowMap(bool checked)
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
        m_dlgOSM->raise();
        m_dlgOSM->activateWindow();
        m_dlgOSM->show();
    }
}

void DlgJIO::onShowGeo(bool checked)
{
    Q_UNUSED(checked)
    QString tile = getTile();

    if (ui->tableWidget->rowCount()<1){
        QMessageBox::information(this, "Info",
                                 "Require at last one GPS locaton",
                                 QMessageBox::Ok);
        return;
    }
    if (m_dlgGeo){

        double lat1 = ui->tableWidget->item(0, GPScols::Latitude)->text().toDouble();
        double lon1 = ui->tableWidget->item(0, GPScols::Longitude)->text().toDouble();

        m_dlgGeo->load(tile , lat1, lon1);
        m_dlgGeo->raise();
        m_dlgGeo->activateWindow();
        m_dlgGeo->show();
    }
}

void DlgJIO::onShow3D(bool checked)
{
    Q_UNUSED(checked)
    qDebug() << "TODO Show 3D plot";

}

void DlgJIO::onToDMS(bool checked)
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
        ui->leSec->setText(QString::number(dms.seconds, 'f', 6));
    }

}

void DlgJIO::onToDegree(bool checked)
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
        ui->leDegree->setText(QString::number(degree, 'f', 6));
    }
}

void DlgJIO::showContextMenu(const QPoint &pos)
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

void DlgJIO::onLoadFinished(bool ok)
{
    if (ok){
        if (m_dlgGeo){
            m_dlgGeo->clearMarker();
            m_dlgGeo->clearPolyLines();
            QString label;
            double lat0;
            double lon0;
            double lat;
            double lon;
            // marker
            for(int row=0;row<ui->tableWidget->rowCount();row++){
                label = ui->tableWidget->item(row, GPScols::PositionName)->text();
                lat = ui->tableWidget->item(row, GPScols::Latitude)->text().toDouble();
                lon = ui->tableWidget->item(row, GPScols::Longitude)->text().toDouble();
                if (row==0){
                    // m_dlgGeo->addMarker(lat, lon, label, Placemark::MarkColor::Red);
                    lat0 = lat;
                    lon0 = lon;
                    m_dlgGeo->addRectangle(QGV::GeoPos(lat0, lon0), QPointF(10.0, 20.0), Qt::red, label);
                }else{
                    // marker
                    //m_dlgGeo->addMarker(lat, lon, label);
                    m_dlgGeo->addRectangle(QGV::GeoPos(lat, lon), QPointF(10.0, 20.0), Qt::yellow, label);
                    // polyLines
                    m_dlgGeo->addPolyline(QGV::GeoPos(lat0, lon0),QGV::GeoPos(lat, lon));
                }
            }
        }
        //old html base map
        // if(m_dlgOSM){
        //     //reload marker
        //     m_dlgOSM->clearMarker();
        //     QString label;
        //     QString lat;
        //     QString lon;
        //     // marker
        //     for(int row=0;row<ui->tableWidget->rowCount();row++){
        //         label = ui->tableWidget->item(row,0)->text();
        //         lat = ui->tableWidget->item(row,1)->text();
        //         lon = ui->tableWidget->item(row,2)->text();
        //         if (row==0){
        //             m_dlgOSM->addMarker(lat, lon, label, "marker0");
        //         }else{
        //             m_dlgOSM->addMarker(lat, lon, label);
        //         }
        //     }
        // }
    }
}

void DlgJIO::onTileAvailable(bool ok)
{
    ui->pbShowMap->setEnabled(ok);
    ui->pbShowGeo->setEnabled(ok);
}

void DlgJIO::onCheckTileFinished()
{
    if (reply->error() == QNetworkReply::NoError) {
        // QByteArray response = reply->readAll();
        // qDebug() << "Response:" << response;
        emit TileAvailable(true);
    } else {
        QString errmsg= reply->errorString();
        qDebug() << "onCheckTileFinished: error: "  << errmsg;
        QMessageBox::warning(this, "Error", errmsg, QMessageBox::Ok);
        emit TileAvailable(false);
    }
}

void DlgJIO::onCheckTileErrorOccurred(QNetworkReply::NetworkError errorcode)
{
    qDebug() << errorcode << " onCheckTileErrorOccurred: " << reply->errorString();
}

void DlgJIO::handleButtonClicked(int row, int col)
{
    qDebug() << "handleButtonClicked: " << QString::number(row)
             << " col:" << QString::number(col);
    //open AIP module setting dialog, after setting, set correct AIP value back to cell
    m_dlgaip->setRowCol(row, col);
    QTableWidgetItem *item = ui->tableWidget->item(row, col);
    if (item){
        m_dlgaip->loadData(item->text());
    }
    m_dlgaip->show();
}

void DlgJIO::onAcceptedAIP()
{
    // qDebug() << "onAcceptedAIP:" << sender();
    QJsonObject data = m_dlgaip->getData();
    qDebug() << "onAcceptedAIP:" << data.value("moduletype").toInt()
             << " x:" << data.value("X_Offset").toDouble()
             << " y:" << data.value("Y_Offset").toDouble()
             << " z:" << data.value("Z_Offset").toDouble();

    //TODO: update to row/col
    // DlgAIP daip = static_cast<DlgAIP>(sender());
    // qDebug() << "onAcceptedAIP: ModuleType: " << daip.getModuleType();
}

void DlgJIO::onUpdateData(int row, int col, QString data)
{
    QTableWidgetItem *item = ui->tableWidget->item(row, col);
    item->setText(data);
}

void DlgJIO::onLoad(QString filename)
{
    // qDebug() << "onLoad file:" << filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << file.errorString();
        return; // Or handle the error appropriately
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "Failed to parse JSON:" << parseError.errorString();
        return; // Or handle the error appropriately
    }
    if (jsonDoc.isNull()) {
        qDebug() << "QJsonDocument is null after parsing.";
        return; // Or handle the error appropriately
    }

    // Now jsonDoc contains the loaded JSON data, and you can access its content
    // For example, if it's an object:
    if (jsonDoc.isObject()) {
        //clear old contents
        ui->twResult->clearContents();
        ui->twResult->setRowCount(0);
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);

        QJsonObject rootObject = jsonDoc.object();
        // Process the QJsonObject
        QJsonArray addPos = rootObject["positions"].toArray();
        for (QJsonArray::const_iterator it=addPos.constBegin(); it!=addPos.constEnd(); ++it) {
            QJsonObject posdata= it->toObject();
            onAddRow(posdata["name"].toString(), posdata["latitude"].toDouble(),
                     posdata["longitude"].toDouble(), posdata["altitude"].toDouble());
        }
    }
}

bool DlgJIO::onSave(QString filename)
{
    debug("onSave file:" + filename, 6);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        debug("Failed to open file:" + file.errorString(),3);
        return false; // Or handle the error appropriately
    }
    QJsonDocument jsonDoc ;
    QJsonObject rootObject;
    QJsonArray pos;
    QTableWidgetItem *item;
    if (ui->tableWidget->rowCount()>0){
        for(int i=0; i< ui->tableWidget->rowCount(); i++){
            QJsonObject posdata;
            item = ui->tableWidget->item(i, GPScols::PositionName);
            if (item) {
                posdata["name"] = item->text();
            }
            item = ui->tableWidget->item(i, GPScols::Latitude);
            if (item) {
                posdata["latitude"] = item->text().toDouble();
            }
            item = ui->tableWidget->item(i, GPScols::Longitude);
            if (item) {
                posdata["longitude"] = item->text().toDouble();
            }
            item = ui->tableWidget->item(i, GPScols::Altitude);
            if (item) {
                posdata["altitude"] = item->text().toDouble();
            }
            //AIP1
            item = ui->tableWidget->item(i, GPScols::AIP1);
            if (item) {
                qDebug() << "AIP1 data:" << item->text();
            }
            //AIP2
            item = ui->tableWidget->item(i, GPScols::AIP2);
            if (item) {
                qDebug() << "AIP2 data:" << item->text();
            }
            pos.append(posdata);
        }
        rootObject["positions"] = pos;
        jsonDoc.setObject(rootObject);
        QString strJson(jsonDoc.toJson(QJsonDocument::Indented));

        QTextStream out(&file);
        out << strJson;
        out.flush();
    }
    file.close();
    return true;
}

void DlgJIO::debug(QString msg, int lv)
{
    if (lv<=m_debuglv){
        qDebug() << "[DlgGpsCalc]" << msg;
    }
}

void DlgJIO::loadcfg()
{
    m_cfg->beginGroup("GpsCalc");
    // m_cfg->setValue("oldsavepath", m_oldsavepath);
    m_oldsavepath = m_cfg->value("oldsavepath",
                                 QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    m_cfg->endGroup();
}

void DlgJIO::savecfg()
{
    m_cfg->beginGroup("GpsCalc");
    m_cfg->setValue("oldsavepath", m_oldsavepath);
    m_cfg->endGroup();
    m_cfg->sync();

}
