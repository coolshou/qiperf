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
#include <QFileInfo>
#include <QVariantMap>
#include <QVariant>

#include "../src/gps/geotranslate.h"
#include "comm.h"
#include "../src/numberdelegate.h"

#include <QDebug>


DlgJIO::DlgJIO(QSettings *cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgJIO), m_cfg(cfg)
{
    m_debuglv=3;
    m_InquireTimer= new QTimer(this);
    connect(m_InquireTimer, &QTimer::timeout, this, &DlgJIO::onInquireTimerTimeout);
    ui->setupUi(this);
    loadcfg();
    ui->pbShow3D->setVisible(false);
    // ui->pbShowMap->setVisible(false);//html base map. not good to show correct position

    ui->tableWidget->setColumnWidth(GPScols::Latitude, 90);
    ui->tableWidget->setColumnWidth(GPScols::Longitude, 90);
    ui->tableWidget->setColumnWidth(GPScols::Altitude, 60);
    ui->tableWidget->setColumnWidth(GPScols::Heading, 50);
    ui->tableWidget->setColumnWidth(GPScols::Pitch, 50);
    ui->tableWidget->setColumnWidth(GPScols::AIP1, 40);
    ui->tableWidget->setColumnWidth(GPScols::AIP2, 40);
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
    NumberDelegate *dHeadDelegate = new NumberDelegate(NumberDelegate::Double,
                                                      0.0, 359, 1,
                                                      ui->tableWidget);
    ui->tableWidget->setItemDelegateForColumn(GPScols::Heading, dHeadDelegate);
    NumberDelegate *dPitchDelegate = new NumberDelegate(NumberDelegate::Double,
                                                      -90.0, 90, 1,
                                                       ui->tableWidget);
    ui->tableWidget->setItemDelegateForColumn(GPScols::Pitch, dPitchDelegate);

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

    ui->twAIP->setItemDelegateForColumn(AIPcols::Azimuth, dAziDelegate);
    ui->twAIP->setItemDelegateForColumn(AIPcols::Elevation, dElDelegate);
    ui->twAIP->setColumnWidth(AIPcols::BeamDirectionID, 100);
    initAction();
    // m_dlgOSM = new DlgOpenStreetMap();
    // connect(m_dlgOSM, &DlgOpenStreetMap::loadFinished, this, &DlgJIO::onLoadFinished);
    m_dlgGeo = new DlgGeoOSM();
    connect(m_dlgGeo, &DlgGeoOSM::loadFinished, this, &DlgJIO::onLoadFinished);
    connect(m_dlgGeo, &DlgGeoOSM::addPosition, this, &DlgJIO::onAddPosition);
    // connect(this, &DlgJIO::closeAll, m_dlgOSM, &DlgOpenStreetMap::close);
    connect(this, &DlgJIO::closeAll, m_dlgGeo, &DlgGeoOSM::close);
    connect(this, &DlgJIO::highlightItm, m_dlgGeo, &DlgGeoOSM::setItmHighlight);

    connect(ui->pbLoad, &QPushButton::clicked, this, &DlgJIO::onLoadCliecked);
    connect(ui->pbSave, &QPushButton::clicked, this, &DlgJIO::onSaveCliecked);
    connect(ui->pbCalc, &QPushButton::clicked, this, &DlgJIO::onCalcCliecked);
    // connect(ui->pbShowMap, &QPushButton::clicked, this, &DlgJIO::onShowMap);
    connect(ui->pbShowGeo, &QPushButton::clicked, this, &DlgJIO::onShowGeo);
    connect(ui->pbShow3D, &QPushButton::clicked, this, &DlgJIO::onShow3D);
    connect(ui->pbClear, &QPushButton::clicked, m_clearAction, &QAction::triggered);
    connect(ui->pbToDMS, &QPushButton::clicked, this, &DlgJIO::onToDMS);
    connect(ui->pbToDegree, &QPushButton::clicked, this, &DlgJIO::onToDegree);

    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested,
            this, &DlgJIO::showContextMenu);
    connect(ui->tableWidget, &QTableWidget::currentCellChanged,
            this, &DlgJIO::onDeviceCellChanged);
    connect(this, &DlgJIO::TileAvailable, this , &DlgJIO::onTileAvailable);

    isTileAvailable();
    provider = new IpLocationProvider(this);
    connect(provider, &IpLocationProvider::locationReady, this, &DlgJIO::onLocationReady);
    connect(provider, &IpLocationProvider::locationError, this, [](const QString& err) {
        qWarning() << "Location fetch failed:" << err;
    });
    getSelfIpLocation();

    m_dlgaip = new DlgAIP(m_cfg, this);
    connect(m_dlgaip, &DlgAIP::updateData, this, &DlgJIO::onUpdateData);
    connect(m_dlgaip, &DlgAIP::updateModelType, this,
            static_cast<void (DlgJIO::*)(int, int, QString)>(&DlgJIO::onUpdateModelType));
    connect(this, &DlgJIO::closeAll, m_dlgaip, &DlgAIP::close);

    m_dlgset = new DlgSet(this);
    connect(m_dlgset, &DlgSet::updateSetting, this, &DlgJIO::onUpdateSetting);

    connect(ui->pbSet, &QPushButton::clicked, this, &DlgJIO::onSet);
    // keep quire device
    connect(ui->pbInquire, &QPushButton::clicked, this, &DlgJIO::onInquireClicked);
    // Do Optimiz
    connect(ui->pbOptimiz, &QPushButton::clicked, this, &DlgJIO::onOptimizClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DlgJIO::close); // close button click

    //ssh
    mSSHRemoteRunner = new QSsh::SshRemoteProcessRunner(this);
    connect(mSSHRemoteRunner, &QSsh::SshRemoteProcessRunner::connectionError,
            this, &DlgJIO::handleSSHConnectionError);
    connect(mSSHRemoteRunner, SIGNAL(processStarted()),
            SLOT(handleSSHProcessStarted()));
    connect(mSSHRemoteRunner, SIGNAL(readyReadStandardOutput()), SLOT(handleSSHProcessStdout()));
    connect(mSSHRemoteRunner, SIGNAL(readyReadStandardError()), SLOT(handleSSHProcessStderr()));
    connect(mSSHRemoteRunner, SIGNAL(processClosed(int)),
            SLOT(handleSSHProcessClosed(int)));
    m_state = Inactive;
    m_started = false;

    // AIP module
    // AIP - Hanwha
    initHanwha();

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
    if (m_dlgGeo){
        m_dlgGeo->clearAllPlot();
    }
}

QString DlgJIO::getStMotion(QString target)
{
    Q_UNUSED(target)
    //get st_motion info
    QString result="";
    /*
# 檢查 st_motion process 有沒有帶，沒有的話要手動帶
ps | grep st_motion  | grep -v grep
# 帶 st_motion 的 指令
st_motion rc e d e e 100 1 1 2 4 4 uen 3 eun 0 1 100 0 0.5 0 2 3000 &
# 讀 sensor 的值
sensors_call_so
## NOTE: require Calibration
Calibration_status_of_the_Sensors = 2

*/
    if (mControlBy==DlgSet::ControlBy::SSH){

    }
    return result;
}

QString DlgJIO::getGpsInfo(QString target)
{
    // get GPS info
    QString result="";
    //   gps_call_so
    if (mControlBy==DlgSet::ControlBy::SSH){
        m_sshParams.setHost(target);
        mSSHRemoteRunner->run("gps_call_so", m_sshParams);
    }
    return result;
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

void DlgJIO::initHanwha()
{
    connect(ui->pbHanwha, &QPushButton::clicked, this, &DlgJIO::showHanwha);
    mHanwha = new Hanwha();
    mDlgHanwha = new DlgHanwha(mHanwha);
    // connect(mHanwha, &Hanwha::newBeamTableIDs, mDlgHanwha, &DlgHanwha::onNewHanwhaBeamTableIDs);
    connect(mHanwha, &Hanwha::updateBeamTableData, mDlgHanwha, &DlgHanwha::onUpdateHanwhaBeamTableData);
    connect(mHanwha, &Hanwha::updateBeamTypes, mDlgHanwha, &DlgHanwha::onUpdateBeamTypes);
    connect(mHanwha, &Hanwha::updateBeamTypeGroup, mDlgHanwha, &DlgHanwha::onUpdateBeamTypeGroup);
    connect(mHanwha, &Hanwha::updateRefFile, mDlgHanwha, &DlgHanwha::setRefFileName);
    connect(mDlgHanwha, &DlgHanwha::reffilechanged, mHanwha, QOverload<QString>::of(&Hanwha::initBeamData));

    QResource resHanwha(":/AIP/Hanwha.xlsx");
    QString filename = resHanwha.fileName();
    QFile Hanwhafile(":/AIP/Hanwha.xlsx");
    if (Hanwhafile.open(QIODevice::ReadOnly)) {
        qDebug() << "Calling Hanwha initBeamData with QFile...";
        mHanwha->initBeamData(&Hanwhafile);// Pass the address of the QFile object
        Hanwhafile.close(); // Close the file after initBeamData is done
    } else {
        qDebug() << "Failed to open" << filename << "for reading:" << Hanwhafile.errorString();
    }
}

void DlgJIO::showHanwha(bool checked)
{
    Q_UNUSED(checked)
    mDlgHanwha->show();
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
    onAddRow("New", 0.0, 0.0, 0.0, 0.0, 0.0);
}

void DlgJIO::onDelete(bool checked)
{
    Q_UNUSED(checked)
    int iRow = ui->tableWidget->currentRow();//->selectRow();
    qDebug() << "onDelete:" <<  QString::number(iRow);
    ui->tableWidget->removeRow(iRow);
}

void DlgJIO::onAddRow(QString name, double latitude, double longitude,
                      double altitude, double heading, double pitch,
                      QJsonObject aip1, QJsonObject aip2, QString ipaddr)
{
    int iRow = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(iRow);
    QString err = QString("[onAddRow]name: %1 ,latitude: %2 ,longitude: %3 ,altitude: %4 , heading: %5").arg(name,
                           QString::number(latitude, 'f', 6),
                           QString::number(longitude, 'f', 6),
                           QString::number(altitude, 'f', 2),
                           QString::number(heading, 'f', 2));
    debug(err, 5);
    ui->tableWidget->setSortingEnabled(false);
    ui->tableWidget->setItem(iRow, GPScols::PositionName, new QTableWidgetItem(name));
    ui->tableWidget->setItem(iRow, GPScols::Latitude, new QTableWidgetItem(QString::number(latitude, 'f', 6)));
    ui->tableWidget->setItem(iRow, GPScols::Longitude, new QTableWidgetItem(QString::number(longitude, 'f', 6)));
    ui->tableWidget->setItem(iRow, GPScols::Altitude, new QTableWidgetItem(QString::number(altitude, 'f', 2)));
    ui->tableWidget->setItem(iRow, GPScols::Heading, new QTableWidgetItem(QString::number(heading, 'f', 2)));
    ui->tableWidget->setItem(iRow, GPScols::Pitch, new QTableWidgetItem(QString::number(pitch, 'f', 2)));
    //how to hold AIP data?
    QPushButton *btn1 = new QPushButton("...");
    int iCol = static_cast<int>(GPScols::AIP1);
    connect(btn1, &QPushButton::clicked, this, [this, iRow, iCol]() {
        handleButtonClicked(iRow, iCol);
    });
    ui->tableWidget->setCellWidget(iRow, GPScols::AIP1, btn1);
    QTableWidgetItem *aip1item = new QTableWidgetItem("");
    aip1item->setData(Qt::UserRole, aip1.toVariantMap());
    // qDebug() << "AIP1:" << aip1;
    ui->tableWidget->setItem(iRow, GPScols::AIP1, aip1item);
    //TODO: update btn text
    onUpdateModelType(iRow, GPScols::AIP1, aip1.value("moduletype").toInt());
    if (iRow==0){
        QPushButton *btn2 = new QPushButton("...");
        iCol = static_cast<int>(GPScols::AIP2);
        connect(btn2, &QPushButton::clicked, this, [this, iRow, iCol]() {
            handleButtonClicked(iRow, iCol);
        });
        ui->tableWidget->setCellWidget(iRow, GPScols::AIP2, btn2);
        QTableWidgetItem *aip2item = new QTableWidgetItem("");
        aip2item->setData(Qt::UserRole, aip2.toVariantMap());
        // qDebug() << "aip2:" << aip2;
        ui->tableWidget->setItem(iRow, GPScols::AIP2, aip2item);
        //TODO: update btn text
        onUpdateModelType(iRow, GPScols::AIP2, aip2.value("moduletype").toInt());
    }else{
        // disable cell
        QTableWidgetItem *disableItem = new QTableWidgetItem("");
        disableItem->setFlags(disableItem->flags() & ~Qt::ItemIsEnabled); //disabled
        disableItem->setBackground(Qt::lightGray);
        disableItem->setFlags(disableItem->flags() & ~Qt::ItemIsSelectable); //not selectable
        ui->tableWidget->setItem(iRow, GPScols::AIP2, disableItem);
    }
    ui->tableWidget->setItem(iRow, GPScols::IPAddr, new QTableWidgetItem(ipaddr));

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
    QList<double> azbearings;
    // QList<double> elbearings;
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
            VincentyResult vrs = vincentyInverse(lat1, lon1, lat, lon);
            distance = vrs.distance/1000; // m -> KM
            azimuth = vrs.finalBearing;
            if (azimuth>180){
                azimuth2 = azimuth-180;
            }else{
                azimuth2 = azimuth+180;
            }
        }
        if (ui->rbHaversine->isChecked()){
            distance = haversine(lat1, lon1, lat, lon);
            azimuth = calcBearing(lat1, lon1, lat, lon);
            azimuth2 = calcBearing(lat, lon, lat1, lon1);
        }
        azbearings.append(azimuth);
        ui->twResult->setItem(i-1, AZEIcols::Name, new QTableWidgetItem(pos1 + " : " + pos));
        ui->twResult->setItem(i-1, AZEIcols::Distance, new QTableWidgetItem(QString::number(distance)));
        ui->twResult->setItem(i-1, AZEIcols::Azimuth1, new QTableWidgetItem(QString::number(azimuth, 'f', 1)));
        ui->twResult->setItem(i-1, AZEIcols::Azimuth2, new QTableWidgetItem(QString::number(azimuth2, 'f', 1)));

        // totalazimuth = totalazimuth + azimuth;
        el1 = GeoTranslate::calcElevationAngle(altmsl1, altmsl, distance*1000);
        el2 = GeoTranslate::calcElevationAngle(altmsl, altmsl1, distance*1000);
        // qDebug() << " " << QString::number(altmsl1) << " - "  << QString::number(altmsl)
        //          << " distance:" << QString::number(distance)
        //          << " el1:" << QString::number(el1) << " el2:" << QString::number(el2);
        ui->twResult->setItem(i-1, AZEIcols::Elevation1, new QTableWidgetItem(QString::number(el1, 'f', 1)));
        ui->twResult->setItem(i-1, AZEIcols::Elevation2, new QTableWidgetItem(QString::number(el2, 'f', 1)));
        // elbearings.append(el1);
        totalel = totalel + el1;
    }
    double azimuthDegree = averageBearing(azbearings);
    qDebug() << " azimuthDegree:" << QString::number(azimuthDegree);
    ui->leAM7az->setText(QString::number(azimuthDegree, 'f', 1));
    // qDebug() << "totalel:" << QString::number(totalel);
    double elDegree = totalel/ui->twResult->rowCount();
    ui->leAM7el->setText(QString::number(elDegree, 'f', 1));

    //group CM7 by Azimuth2
    QColor lColor = QColor(144, 238, 144); //light green
    QColor rColor = QColor(173, 216, 230); //light blue
    QColor nColor = QColor(Qt::lightGray);
    double relative;
    double cmaz;
    QList<QTableWidgetItem*> cm7rs;
    QList<QTableWidgetItem*> cm7ls;
    for (int iRow=0;iRow<ui->twResult->rowCount();iRow++){
        QTableWidgetItem *itm = ui->twResult->item(iRow, AZEIcols::Azimuth1);
        if (itm){
            cmaz = itm->text().toDouble();// + 180;
            relative = fmod((cmaz - azimuthDegree + 360), 360);
            // qDebug() << "cmaz:" << cmaz << "  relative:" << relative;
            if (relative > 0 && relative < 90){
                //azimuthDegree 的第一象限
                itm->setBackground(QBrush(lColor));
                itm->setToolTip("CM7-FirstQuadrant");
                cm7rs.append(itm);
            }else if (relative > 270 && relative < 360){
                //azimuthDegree 的第四象限
                itm->setBackground(QBrush(rColor));
                itm->setToolTip("CM7-FourthQuadrant");
                cm7ls.append(itm);
            }else{
                qDebug() << "No in Coverage range";
                itm->setBackground(QBrush(nColor));
            }
        }
    }
    // 1. get cm7rs Max and Min value
    // diff = |Max - Min|
    // check diff < 3dB Az BW
    // Max, Min should not over AM7az ± dirBW ± 3dB_AzBW/2
    ui->tableWidget->item(0, GPScols::AIP1); //cyntec or hanwha

    //AM7 AIP1 Az, TODO El
    QList<double> aipRs;
    double aip1az=0;
    double aip1azdiff=0;
    if (cm7rs.length()>1){
        for(auto azitm: cm7rs){
            aipRs.append(azitm->text().toDouble());
        }
        aip1az = averageBearing(aipRs);
    }else if (cm7rs.length()==1){
        aip1az = cm7rs.value(0)->text().toDouble();
    }else {
        qDebug() << "No AIP1 AZ value";
    }
    aip1azdiff = aip1az-azimuthDegree;
    QList<double> aipLs;
    //AM7 AIP2 Az,TODO El
    double aip2az=0;
    double aip2azdiff=0;
    if (cm7ls.length()>1){
        for(auto azitm: cm7ls){
            aipLs.append(azitm->text().toDouble());
        }
        aip2az = averageBearing(aipLs);
    }else if (cm7ls.length()==1){
        aip2az = cm7ls.value(0)->text().toDouble();
    }else {
        qDebug() << "No AIP2 AZ value";
    }
    aip2azdiff = aip2az-azimuthDegree;
    ui->twAIP->setItem(0, AIPcols::Azimuth,
                       new QTableWidgetItem(QString::number(aip1az)));
    //TODO AIP1 el
    ui->twAIP->setItem(0, AIPcols::Azdiff,
                       new QTableWidgetItem(QString::number(aip1azdiff)));
    ui->twAIP->setItem(1, AIPcols::Azimuth,
                       new QTableWidgetItem(QString::number(aip2az)));
    //TODO AIP2 el
    ui->twAIP->setItem(1, AIPcols::Azdiff,
                       new QTableWidgetItem(QString::number(aip2azdiff)));
}

void DlgJIO::onSet(bool checked)
{
    Q_UNUSED(checked)
    //show config of ssh username/password
    if (m_dlgset){
        m_dlgset->setSSH(mSshUsername, mSshPassword);
        m_dlgset->setWeb(mWebusername, mWebpassword);
        m_dlgset->show();
    }
}

void DlgJIO::onInquireClicked(bool checked)
{
    if (checked){
        m_sshParams.setUserName(mSshUsername);
        m_sshParams.setPassword(mSshPassword);

        qDebug() << "Inquire start after 1 sec";
        m_InquireTimer->start(3000);// 1sec

    }else{
        if (m_InquireTimer->isActive()){
            m_InquireTimer->stop();
        }
    }
}

void DlgJIO::onOptimizClicked(bool checked)
{
    Q_UNUSED(checked)
    qDebug() <<"//do Optimiz to get All device's Beam Direction ID/ Att value";
}

void DlgJIO::onInquireTimerTimeout()
{

    if (ui->tableWidget->rowCount()>0){
        qDebug() << "do Inquire";
        for (int row=0; row < ui->tableWidget->rowCount(); row++){
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            //IPAddr
            QTableWidgetItem *itm= ui->tableWidget->item(row, GPScols::IPAddr);
            if (itm){
                if (!itm->text().isEmpty()){
                    qDebug() << "onInquireTimerTimeout //TODO Inquire:" << itm->text();
                    //Use ssh
                    if (mControlBy == DlgSet::ControlBy::SSH){
                        qDebug() << "control by SSH";
                        getGpsInfo(itm->text());

                    }
                }else{
                    qDebug() << "No IPAddr at row:" << row << ", col:" << static_cast<int>(GPScols::IPAddr);
                }
            }else{
                qDebug() << "No item at row:" << row << ", col:" << static_cast<int>(GPScols::IPAddr);
            }
        }
    }else{
        qDebug() << "No item of device";
        m_InquireTimer->stop();
    }
}

// void DlgJIO::onShowMap(bool checked)
// {
//     Q_UNUSED(checked)
//     QString errmsg ="";
//     //TODO: check openstreetmap can be accessable
//     QString tile = getTile();
//     if (tile.isEmpty()){
//         errmsg = "Require OpenStreetMapTile set eq: https://tile.openstreetmap.org/{z}/{x}/{y}.png";
//         qDebug() << errmsg;
//         QMessageBox::warning(this, "ERROR", errmsg, QMessageBox::Ok);
//         return;
//     }

//     if (ui->tableWidget->rowCount()<1){
//         QMessageBox::information(this, "Info",
//                                  "Require at last one GPS locaton",
//                                  QMessageBox::Ok);
//         return;
//     }

//     if (m_dlgOSM){
//         // QString pos1 = ui->tableWidget->item(0,0)->text();
//         QString lat1 = ui->tableWidget->item(0,1)->text();
//         QString lon1 = ui->tableWidget->item(0,2)->text();
//         m_dlgOSM->load(tile , lat1, lon1);
//         m_dlgOSM->raise();
//         m_dlgOSM->activateWindow();
//         m_dlgOSM->show();
//     }
// }

void DlgJIO::onShowGeo(bool checked)
{
    Q_UNUSED(checked)
    QString tile = getTile();

    if (ui->tableWidget->rowCount()>1){
        onCalcCliecked(true);
    }

    if (m_dlgGeo){
        m_dlgGeo->clearAllPlot();
        double lat1 = mIpLocation.latitude;
        double lon1 = mIpLocation.longitude;
        if (ui->tableWidget->rowCount()>0){
            lat1 = ui->tableWidget->item(0, GPScols::Latitude)->text().toDouble();
            lon1 = ui->tableWidget->item(0, GPScols::Longitude)->text().toDouble();
        }
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

void DlgJIO::onDeviceCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)
    QTableWidgetItem *itm = ui->tableWidget->item(currentRow, GPScols::PositionName);
    if (itm){
        QString lable= itm->text();
        emit highlightItm(lable);
    }
}

void DlgJIO::onLoadFinished(bool ok)
{
    if (ok){
        if (m_dlgGeo){
            QTableWidgetItem *itm=nullptr;
            QString label;
            double lat0;
            double lon0;
            double lat;
            double lon;
            double azdeg;
            QRgb rgb1 = 0xFFDC7000; //淺棕色
            QRgb rgbaz = 0xFF6CBCF1; //中等亮度的藏青色
            QGV::GeoPos am7;
            QGV::GeoPos cm;
            // marker & phy arrow line
            for(int row=0;row<ui->tableWidget->rowCount();row++){
                label = ui->tableWidget->item(row, GPScols::PositionName)->text();
                lat = ui->tableWidget->item(row, GPScols::Latitude)->text().toDouble();
                lon = ui->tableWidget->item(row, GPScols::Longitude)->text().toDouble();
                if (row==0){
                    // m_dlgGeo->addMarker(lat, lon, label, Placemark::MarkColor::Red);
                    lat0 = lat;
                    lon0 = lon;
                    am7 = QGV::GeoPos{lat0, lon0};
                    m_dlgGeo->addRectangle(am7, QPointF(20.0, 10.0), Qt::red, label);
                    //draw Main Arrow line
                    if (!ui->leAM7az->text().isEmpty()){
                        m_dlgGeo->addArrowLine(am7,
                                               ui->leAM7az->text().toDouble(),
                                               100,
                                               QColor(Qt::red));
                    }
                }else{
                    // marker
                    cm = QGV::GeoPos{lat, lon};
                    //m_dlgGeo->addMarker(lat, lon, label);
                    m_dlgGeo->addRectangle(cm, QPointF(20.0, 10.0), Qt::yellow, label);
                    // polyLines
                    m_dlgGeo->addLinkline(am7, cm);
                    //draw Arrow line
                    itm = ui->twResult->item(row-1, AZEIcols::Azimuth2);
                    if (itm){
                        azdeg = itm->text().toDouble();
                        m_dlgGeo->addArrowLine(cm, azdeg, 100, QColor(rgb1));
                    }
                }
            }
            //final signal Azimuth
            if (ui->twAIP->rowCount()>0){
                for(int row=0;row<ui->twAIP->rowCount();row++){
                    itm = ui->twAIP->item(row, AIPcols::Azimuth);
                    if (itm){
                        if (!itm->text().isEmpty()){
                            azdeg = itm->text().toDouble();
                            m_dlgGeo->addArrowLine(am7, azdeg, 200, QColor(rgbaz));
                        }
                    }
                }
            }
        }
    }
}

void DlgJIO::onAddPosition(QString label, double lat, double lon)
{
    onAddRow(label, lat, lon, 0.0, 0.0, 0.0);
}

void DlgJIO::onTileAvailable(bool ok)
{
    // ui->pbShowMap->setEnabled(ok);
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
    // qDebug() << "handleButtonClicked: " << QString::number(row) << " col:" << QString::number(col);
    //open AIP module setting dialog, after setting, set correct AIP value back to cell
    m_dlgaip->setRowCol(row, col);
    QTableWidgetItem *item = ui->tableWidget->item(row, col);
    if (item){
        QVariant v = item->data(Qt::UserRole);
        qDebug() << "AIP data:"
                 << QString::number(row) << "," <<  QString::number(col)
                 << " = " << v;
        if (v.canConvert<QJsonObject>()){
            // QJsonObject j = v.toObject();
            m_dlgaip->loadData(qvariant_cast<QJsonObject>(v));
        }else{
            qDebug() << "data from " << item << " can not convert to QJsonObject format";
        }
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

void DlgJIO::onUpdateData(int row, int col, QJsonObject data)
{
    QTableWidgetItem *item = ui->tableWidget->item(row, col);
    qDebug() << row << "," << col << " DlgJIO::onUpdateData" <<data;
    item->setData(Qt::UserRole, data.toVariantMap());
    onUpdateModelType(row, col, data.value("moduletype").toInt());
}

void DlgJIO::onUpdateModelType(int row, int col, QString smodel)
{
    QWidget *cell = ui->tableWidget->cellWidget(row, col);
    if (cell != nullptr){
        QPushButton *pb = static_cast<QPushButton*>(cell);
        // qDebug() << "onUpdateModelType: " << QString::number(row) << ","
        //          <<  QString::number(col) << " model:" << smodel;
        pb->setText(smodel);
    } else {
        qDebug() << "no QPushButton in cell "  << QString::number(row) << ","
                 <<  QString::number(col) ;
    }
}

void DlgJIO::onUpdateModelType(int row, int col, int model)
{
    if (static_cast<AIP::ModuleType>(model) == AIP::ModuleType::Cyntec){
        onUpdateModelType(row, col, "C");
    }else if (static_cast<AIP::ModuleType>(model) == AIP::ModuleType::Hanwha){
        onUpdateModelType(row, col, "H");
    }else{
        onUpdateModelType(row, col, "...");
    }
}

void DlgJIO::onUpdateSetting(QString sshusername, QString sshpassword,
                             QString webusername, QString webpassword,
                             DlgSet::ControlBy ctl)
{
    mSshUsername = sshusername;
    mSshPassword = sshpassword;
    mWebusername = webusername;
    mWebpassword = webpassword;
    mControlBy = ctl;
}

void DlgJIO::onLocationReady(const IpLocation &location)
{
    // qDebug() << "Coordinates:" << location.latitude << "," << location.longitude;
    mIpLocation = location;
}

void DlgJIO::handleSSHConnectionError()
{
    qDebug() << "SSHConnectionError: " << mSSHRemoteRunner->lastConnectionErrorString();
}

void DlgJIO::handleSSHProcessStarted()
{
    if (m_started)
    {
        qDebug() << "Error: Received started() signal again.";
    }
    else
    {
        m_started = true;
        m_remoteStdout.clear();
        m_remoteStderr.clear();
        // if (m_state == TestingCrash)
        // {
        //     QSsh::SshRemoteProcessRunner *const killer = new QSsh::SshRemoteProcessRunner(this);
        //     //TODO: other platform, eq: windows
        //     killer->run("pkill -9 sleep", m_sshParams);
        // }
        // else if (m_state == TestingIoDevice)
        // {
        //     connect(m_catProcess.data(), SIGNAL(readyRead()), SLOT(handleReadyRead()));
        //     m_textStream.reset(new QTextStream(m_catProcess.data()));
        //     *m_textStream << testString();
        //     m_textStream->flush();
        // }
    }
}

void DlgJIO::handleSSHProcessStdout()
{
    if (!m_started)
    {
        qDebug() << "Error: Remote output from non-started process.";
    }
    else if (m_state != TestingSuccess && m_state != TestingTerminal)
    {
        qDebug() << "Error: Got remote standard output in state " << m_state;
    }
    else
    {
        m_remoteStdout += mSSHRemoteRunner->readAllStandardOutput();
    }
}

void DlgJIO::handleSSHProcessStderr()
{
    if (!m_started)
    {
        qDebug() << "Error: Remote error output from non-started process.";
    }
    else if (m_state == TestingSuccess)
    {
        qDebug() << "Error: Unexpected remote standard error output.";
    }
    else
    {
        m_remoteStderr += mSSHRemoteRunner->readAllStandardError();
    }
}

void DlgJIO::handleSSHProcessClosed(int exitStatus)
{
    switch (exitStatus)
    {
    case QSsh::SshRemoteProcess::NormalExit:
        if (!m_started)
        {
            qDebug() << "Error: Process exited without starting." ;
            return;
        }
        switch (m_state)
        {
        case TestingSuccess:
        {
            const int exitCode = mSSHRemoteRunner->processExitCode();
            if (exitCode != 0)
            {
                qDebug() << "Error: exit code is " << exitCode
                          << ", expected zero." ;
                return;
            }
            if (m_remoteStdout.isEmpty())
            {
                qDebug() << "Error: Command did not produce output.";
                return;
            }
            qDebug() << "\n" << QString::fromUtf8(m_remoteStdout) << "\n";
            // std::cout << "Ok.\nTesting unsuccessful remote process... " << std::flush;
            // m_state = TestingFailure;
            // m_started = false;
            // m_timeoutTimer->start();
            // mSSHRemoteRunner->run("top -n 1", m_sshParams); // Does not succeed without terminal.
            // m_timeoutTimer->stop();
            // QCoreApplication::exit(EXIT_SUCCESS);
            break;
        }
        case TestingFailure:
        {
            const int exitCode = mSSHRemoteRunner->processExitCode();
            if (exitCode == 0)
            {
                qDebug() << "Error: exit code is zero, expected non-zero.";
                return;
            }
            if (m_remoteStderr.isEmpty())
            {
                qDebug() << "Error: Command did not produce error output.";
                return;
            }

            qDebug() << "Ok.\nTesting crashing remote process... ";
            m_state = TestingCrash;
            m_started = false;
            // m_timeoutTimer->start();
            mSSHRemoteRunner->run("/bin/sleep 100", m_sshParams);
            break;
        }
        case TestingCrash:
            if (mSSHRemoteRunner->processExitCode() == 0)
            {
                qDebug() << "Error: Successful exit from process that was "
                             "supposed to crash.";
            }
            else
            {
                qDebug() << "// Some shells (e.g. mksh) don't report 'killed', but just a non-zero exit code.";
                // handleSuccessfulCrashTest();
            }
            break;
        case TestingTerminal:
        {
            const int exitCode = mSSHRemoteRunner->processExitCode();
            if (exitCode != 0)
            {
                qDebug() << "Error: exit code is " << exitCode
                          << ", expected zero." ;
                return;
            }
            if (m_remoteStdout.isEmpty())
            {
                qDebug() << "Error: Command did not produce output.";

                return;
            }
            qDebug() << "Ok.\nTesting I/O device functionality... ";
            // m_state = TestingIoDevice;
            // m_sshConnection = new SshConnection(m_sshParams);
            // connect(m_sshConnection, SIGNAL(connected()), SLOT(handleConnected()));
            // connect(m_sshConnection, SIGNAL(error(QSsh::SshError)),
            //         SLOT(handleConnectionError()));
            // m_sshConnection->connectToHost();
            // m_timeoutTimer->start();
            break;
        }
        case TestingIoDevice:
            qDebug() << "TestingIoDevice";
            // if (m_catProcess->exitCode() == 0)
            // {
            //     qDebug() << "Error: Successful exit from process that was supposed to crash.";

            // }
            // else
            // {
            //     handleSuccessfulIoTest();
            // }
            break;
        case TestingProcessChannels:
            if (m_remoteStderr.isEmpty())
            {
                qDebug() << "Error: Did not receive readyReadStderr()." ;
                return;
            }
            // if (m_remoteData != StderrOutput)
            // {
            //     qDebug() << "Error: Expected output '" << StderrOutput.data() << "', received '"
            //               << m_remoteData.data() << "'." ;

            //     return;
            // }
            qDebug() << "Ok.\nAll tests succeeded." ;
            // QCoreApplication::quit();
            break;
        case Inactive:
            Q_ASSERT(false);
        }
        break;
    case QSsh::SshRemoteProcess::FailedToStart:
        if (m_started)
        {
            qDebug() << "Error: Got 'failed to start' signal for process "
                         "that has not started yet.";
        }
        else
        {
            qDebug() << "Error: Process failed to start." ;
        }
        break;
    case QSsh::SshRemoteProcess::CrashExit:
        switch (m_state)
        {
        case TestingCrash:
            qDebug() << "handleSuccessfulCrashTest();";
            break;
        case TestingIoDevice:
            qDebug() << "handleSuccessfulIoTest();";
            break;
        default:
            qDebug() << "Error: Unexpected crash." ;
            return;
        }
    }
}

double DlgJIO::averageBearing(const QList<double> &bearings)
{
    if (bearings.isEmpty()) return -1.0; // 或者 return NaN

    double sumX = 0.0;
    double sumY = 0.0;

    for (double angle : bearings) {
        sumX += qCos(qDegreesToRadians(angle));
        sumY += qSin(qDegreesToRadians(angle));
    }

    double avgRad = qAtan2(sumY, sumX);
    double avgDeg = qRadiansToDegrees(avgRad);
    if (avgDeg < 0.0) avgDeg += 360.0;

    return avgDeg;
}

void DlgJIO::getSelfIpLocation()
{
    provider->fetchLocation();

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
        QJsonObject sshobj = rootObject.value("ssh").toObject();
        mSshUsername = sshobj.value("username").toString();
        mSshPassword = sshobj.value("password").toString();
        QJsonObject webobj = rootObject.value("web").toObject();
        mWebusername = webobj.value("username").toString();
        mWebpassword = webobj.value("password").toString();

        QJsonArray addPos = rootObject.value("positions").toArray();
        for (QJsonArray::const_iterator it=addPos.constBegin(); it!=addPos.constEnd(); ++it) {
            QJsonObject posdata= it->toObject();
            onAddRow(posdata.value("name").toString(), posdata.value("latitude").toDouble(),
                     posdata.value("longitude").toDouble(), posdata.value("altitude").toDouble(),
                     posdata.value("heading").toDouble(), posdata.value("pitch").toDouble(),
                     posdata.value("AIP1").toObject(), posdata.value("AIP2").toObject(),
                     posdata.value("IPAddr").toString());
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
    //ssh
    QJsonObject sshObj;
    sshObj["username"] = mSshUsername;
    sshObj["password"] = mSshPassword;
    rootObject["ssh"] = sshObj;
    //web
    QJsonObject webObj;
    webObj["username"] = mWebusername;
    webObj["password"] = mWebpassword;
    rootObject["web"] = webObj;
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
            item = ui->tableWidget->item(i, GPScols::Heading);
            if (item) {
                posdata["heading"] = item->text().toDouble();
            }
            item = ui->tableWidget->item(i, GPScols::Pitch);
            if (item) {
                posdata["pitch"] = item->text().toDouble();
            }
            //AIP1
            item = ui->tableWidget->item(i, GPScols::AIP1);
            if (item) {
                QVariant varAIP1 =  item->data(Qt::UserRole);
                if (varAIP1.canConvert<QVariantMap>()){
                    qDebug() << "AIP1 data:" << varAIP1.toMap() ;
                    posdata["AIP1"] = QJsonObject::fromVariantMap(varAIP1.toMap());
                }else{
                    qDebug() << "Wrong API1 data:";
                }
            }
            //AIP2
            item = ui->tableWidget->item(i, GPScols::AIP2);
            if (item) {
                QVariant varAIP2 =  item->data(Qt::UserRole);
                if (varAIP2.canConvert<QVariantMap>()){
                    qDebug() << "AIP2 data:" << varAIP2.toMap() ;
                    posdata["AIP2"] = QJsonObject::fromVariantMap(varAIP2.toMap());
                }else{
                    qDebug() << "Row:" << QString::number(i) << " Wrong API2 data";
                }
            }
            item = ui->tableWidget->item(i, GPScols::IPAddr);
            if (item) {
                posdata["IPAddr"] = item->text();
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
        qDebug() << "[DlgJIO]" << msg;
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

// double DlgJIO::bearing(double lat1, double lon1, double lat2, double lon2)
// {
//     double p1 = qDegreesToRadians(lat1);
//     double p2 = qDegreesToRadians(lat2);
//     double dl = qDegreesToRadians(lon2 - lon1);

//     double y = sin(dl) * cos(p2);
//     double x = cos(p1) * sin(p2) - sin(p1) * cos(p2) * cos(dl);
//     double theta = atan2(y, x);
//     double bearingDeg = fmod(qRadiansToDegrees(theta) + 360.0, 360.0);
//     return bearingDeg;
// }
