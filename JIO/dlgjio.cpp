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
#include "../src/wsclient.h"

#include "comm.h"
#include "jiocmd.h"
#include "myfunc.h"

#include "../src/numberdelegate.h"

#include <QDebug>


DlgJIO::DlgJIO(QSettings *cfg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgJIO), m_cfg(cfg)
{
    m_debuglv=3;
    mHeaderResult = QStringList() << "P1 : P2" << "Distance(KM)"
                                  << "P1 Az(°)" << "P2 Az(°)" << "P1 El(°)" << "P2 El(°)"
                                  << "P2\nAz Diff" << "P2\nEl Diff" << "P2\nBeamDir ID";
    mHeaderAIP = QStringList() << "Azimuth(°)" << "Elevation(°)" << "Azdiff" << "BeamDir ID";
    mHeaderHanwha = QStringList() << "BFTx1\nAtt" << "BFTx2\nAtt" << "Tx1\nAtt" << "Tx2\nAtt"
                    << "BFRx1\nAtt" << "BFRx2\nAtt" << "Rx1\nAtt" << "Rx2\nAtt"
                    << "RxLan\nAtt";
    jiocmdObj = QJsonObject();
    m_InquireTimer= new QTimer(this);
    connect(m_InquireTimer, &QTimer::timeout, this, &DlgJIO::onInquireTimerTimeout);
    ui->setupUi(this);
    loadcfg();
    ui->pbShow3D->setVisible(false);
    // ui->pbShowMap->setVisible(false);//html base map. not good to show correct position
    initTableWidget();
    initAction();
    // m_dlgOSM = new DlgOpenStreetMap();
    // connect(m_dlgOSM, &DlgOpenStreetMap::loadFinished, this, &DlgJIO::onLoadFinished);
    m_dlgGeo = new DlgGeoOSM();
    connect(m_dlgGeo, &DlgGeoOSM::loadFinished, this, &DlgJIO::onLoadFinished);
    connect(m_dlgGeo, &DlgGeoOSM::addPosition, this, &DlgJIO::onAddPosition);
    // connect(this, &DlgJIO::closeAll, m_dlgOSM, &DlgOpenStreetMap::close);
    connect(this, &DlgJIO::closeAll, m_dlgGeo, &DlgGeoOSM::close);
    connect(this, &DlgJIO::highlightItm, m_dlgGeo, &DlgGeoOSM::setItmHighlight);
    connect(this, &DlgJIO::deleteItm, m_dlgGeo, &DlgGeoOSM::onDeleteItm);

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
    connect(this, &DlgJIO::closeAll, m_dlgset, &DlgSet::close);
    mDlgBeamCmd= new DlgBeamCmd(this);
    connect(this,&DlgJIO::addBeamIDCmd, mDlgBeamCmd, &DlgBeamCmd::onAddBeamIDCmd);
    connect(this,&DlgJIO::addCMBeamIDCmd, mDlgBeamCmd, &DlgBeamCmd::onAddCMBeamIDCmd);
    connect(this,&DlgJIO::clearBeamIDCmd, mDlgBeamCmd, &DlgBeamCmd::clear);
    connect(this,&DlgJIO::clearCMBeamIDCmd, mDlgBeamCmd, &DlgBeamCmd::clearCM);
    connect(this, &DlgJIO::closeAll, mDlgBeamCmd, &DlgBeamCmd::close);
    mDlgOptimize = new DlgOptimize(this);
    connect(this, &DlgJIO::closeAll, mDlgOptimize, &DlgOptimize::close);
    //ssh
    mSSHRemoteRunner = new QSsh::SshRemoteProcessRunner(this);
    connect(mSSHRemoteRunner, &QSsh::SshRemoteProcessRunner::connectionError,
            this, &DlgJIO::handleSSHConnectionError);
    connect(mSSHRemoteRunner, &QSsh::SshRemoteProcessRunner::processStarted,
            this, &DlgJIO::handleSSHProcessStarted);
    connect(mSSHRemoteRunner, &QSsh::SshRemoteProcessRunner::readyReadStandardOutput,
            this, &DlgJIO::handleSSHProcessStdout);
    connect(mSSHRemoteRunner, &QSsh::SshRemoteProcessRunner::readyReadStandardError,
            this, &DlgJIO::handleSSHProcessStderr);
    connect(mSSHRemoteRunner, &QSsh::SshRemoteProcessRunner::processClosed,
            this, &DlgJIO::handleSSHProcessClosed);
    m_state = Inactive;
    m_started = false;

    // AIP module
    // AIP - Hanwha
    initHanwha();
    // AIP - Cyntec
    initCyntec();
    initCmds();

    connect(this, &DlgJIO::requestExec, this, &DlgJIO::doRequestExec);
    connect(this, &DlgJIO::startOptimiz, this, &DlgJIO::onStartOptimiz);
    connect(this, &DlgJIO::stopOptimiz, this, &DlgJIO::onStopOptimiz);

    initResultHeader(AIP::ModuleType::Hanwha);
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
    if (ui->twAIP->rowCount()>0){
        ui->twAIP->clearContents();
        ui->twAIP->setRowCount(2);
    }
    if (m_dlgGeo){
        m_dlgGeo->clearAllPlot();
    }
    ui->leAM7az->setText("");
    ui->leAM7el->setText("");
    emit clearBeamIDCmd();
}

QString DlgJIO::getStMotion(QString target)
{
    //TODO: check sensor Calibration
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

QString DlgJIO::getGpsInfo(QString refrow, QString target)
{
    // get GPS info
    QString result="";
    if (mControlBy==DlgSet::ControlBy::SSH){
        //TODO:
        m_sshParams.setHost(target);
        mSSHRemoteRunner->run("gps_call_so", m_sshParams);
    }else if (mControlBy==DlgSet::ControlBy::QIPERFD){
        QString cmd = QString("%1:%2").arg(JIO_GET_GPS, jiocmdObj.value(JIO_GET_GPS).toString());
        // qDebug() << "JIO_GET_GPS=" << target << " cmd=> "  << cmd;
        emit requestExec(target, refrow, cmd);
    }
    return result;
}

QString DlgJIO::getSensorInfo(QString refrow, QString target)
{
    // get Sensor info
    QString result="";
    //   sensor_call_so
    if (mControlBy==DlgSet::ControlBy::SSH){
        //TODO:

    }else if (mControlBy==DlgSet::ControlBy::QIPERFD){
        QString cmd = QString("%1:%2").arg(JIO_GET_SENSORS, jiocmdObj.value(JIO_GET_SENSORS).toString());
        // qDebug() << "JIO_GET_SENSORS=" << target << " cmd=> " << cmd;
        emit requestExec(target, refrow, cmd);
    }
    return result;
}

void DlgJIO::getAPInfo(QString refrow, QString target)
{
    if (mControlBy==DlgSet::ControlBy::SSH){
        //TODO: getAPInfo (RSSI/SNR/MCS) by ssh

    }else if (mControlBy==DlgSet::ControlBy::QIPERFD){
        QString cmd = QString("%1:%2").arg(JIO_GET_AP_INFO, jiocmdObj.value(JIO_GET_AP_INFO).toString());
        // qDebug() << "JIO_GET_GPS=" << target << " cmd=> "  << cmd;
        emit requestExec(target, refrow, cmd);
    }
}

AIP::ModuleType DlgJIO::getModuleType(int row, int col)
{
    if ((row<0) || (row >= ui->tableWidget->rowCount())){
        qDebug() << "getModuleType row out of range";
        return AIP::ModuleType::Unknown;
    }
    if ((col != static_cast<int>(GPScols::AIP1))&
        (col != static_cast<int>(GPScols::AIP2))){
        qDebug() << "getModuleType col out of range";
        return AIP::ModuleType::Unknown;
    }
    QTableWidgetItem *itm= ui->tableWidget->item(row, col);
    QVariant variant= itm->data(Qt::UserRole);
    // qDebug() << "getModuleType variant:" << variant;
    if (variant.canConvert<QJsonObject>()){
        QJsonObject obj = variant.toJsonObject();
        // qDebug() << "row:" << QString::number(row)
        //          << " col:" << QString::number(col)
        //          << " moduletype:" << obj;
        return static_cast<AIP::ModuleType>(obj.value("moduletype").toInt());
    }else{
        qDebug() << "getModuleType convert to json fail: " << variant;
        return AIP::ModuleType::Unknown;
    }
}

QJsonObject DlgJIO::createInitData()
{
    //create Init Data for Optimize use
    QJsonObject rootObject;
    QJsonObject aipObj;
    rootObject["LocalAddr"]= ui->leLocalAddr->text();
    rootObject["ControlBy"]= mControlBy;
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
            if (i==0){
                posdata["type"]=0; // AP (AM7)
            }else{
                posdata["type"]=1; // client (CM7)
            }
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
                    // qDebug() << "AIP1 data:" << varAIP1.toMap() ;
                    aipObj = QJsonObject::fromVariantMap(varAIP1.toMap());
                    if (i==0){
                        if (ui->twAIP->rowCount()>0){
                            item = ui->twAIP->item(0, AIPcols::BeamDirectionID);
                            if (item){
                                aipObj["BeamID"] = item->text().toInt();
                            }
                        }
                    }else{
                        if (ui->twResult->rowCount()>0){
                            item = ui->twResult->item(i-1, AZEIcols::P2AzDiff);
                            if (item){
                                aipObj["AzDiff"] = item->text().toDouble();
                            }
                            item = ui->twResult->item(i-1, AZEIcols::P2ElDiff);
                            if (item){
                                aipObj["ElDiff"] = item->text().toDouble();
                            }
                            item = ui->twResult->item(i-1, AZEIcols::BeamDirID);
                            if (item){
                                aipObj["BeamID"] = item->text().toInt();
                            }
                        }
                    }
                    posdata["AIP1"] = aipObj;
                }else{
                    qDebug() << "Wrong API1 data:";
                }
            }
            //AIP2
            item = ui->tableWidget->item(i, GPScols::AIP2);
            if (item) {
                QVariant varAIP2 =  item->data(Qt::UserRole);
                if (varAIP2.canConvert<QVariantMap>()){
                    // qDebug() << "AIP2 data:" << varAIP2.toMap() ;
                    aipObj = QJsonObject::fromVariantMap(varAIP2.toMap());
                    if (i==0){
                        if (ui->twAIP->rowCount()>1){
                            item = ui->twAIP->item(1, AIPcols::BeamDirectionID);
                            if (item){
                                aipObj["BeamID"] = item->text().toInt();
                            }
                        }
                    }
                    posdata["AIP2"] = aipObj;
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
    }
    return rootObject;
}

void DlgJIO::onRequestResult(QString refrow, QString serveraddress, QString cmd, QString msg)
{
    Q_UNUSED(serveraddress)
    // qDebug() << "onRequestResult refrow:" << refrow << " from: " << serveraddress
    //          << " cmd: " << cmd << " msg: " << msg;
    QJsonParseError error;
    QJsonDocument doc;
    if (cmd.contains(JIO_GPS_DATA)){
        doc=QJsonDocument::fromJson(msg.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            qDebug() << "JIO_GPS_DATA: " << msg;
            QJsonObject obj = doc.object();
            QTableWidgetItem *itm;
            foreach (QString key, obj.keys()){
                if (key.contains("Latitude:")){
                    itm = ui->tableWidget->item(refrow.toInt(), GPScols::Latitude);
                    itm->setText(QString::number(obj.value(key).toDouble(), 'f', 6));
                }
                if (key.contains("Longitude:")){
                    itm = ui->tableWidget->item(refrow.toInt(), GPScols::Longitude);
                    itm->setText(QString::number(obj.value(key).toDouble(), 'f', 6));
                }
                if (key.contains("Altitude")){
                    itm = ui->tableWidget->item(refrow.toInt(), GPScols::Altitude);
                    itm->setText(QString::number(obj.value(key).toDouble(), 'f', 2));
                }
                // if (key.contains("Heading")){
                //     itm = ui->tableWidget->item(refrow.toInt(), GPScols::Heading);
                //     itm->setText(QString::number(obj.value(key).toDouble(), 'f', 2));
                // }
            }
        }else{
            qDebug() << "Wrong format of JIO_GPS_DATA msg:(" << error.errorString() << ")\n";
        }
    }else if (cmd.contains(JIO_SENSORS_DATA)){
        doc=QJsonDocument::fromJson(msg.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            qDebug() << "JIO_SENSORS_DATA: " << msg;
            QJsonObject obj = doc.object();
            QTableWidgetItem *itm;
            foreach (QString key, obj.keys()){
                if (key.contains("out_heading:")){
                    itm = ui->tableWidget->item(refrow.toInt(), GPScols::Heading);
                    itm->setText(QString::number(obj.value(key).toDouble(), 'f', 2));
                }
                if (key.contains("out_rotation[1]")){
                    itm = ui->tableWidget->item(refrow.toInt(), GPScols::Pitch);
                    itm->setText(QString::number(obj.value(key).toDouble(), 'f', 2));
                }
            }
        }else{
            qDebug() << "Wrong format of JIO_SENSORS_DATA msg:(" << error.errorString() << ")\n";
        }
    }else if (cmd.contains(JIO_AP_INFO)){
        doc=QJsonDocument::fromJson(msg.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            qDebug() << "JIO_AP_INFO: " << msg;
            QJsonObject obj = doc.object();
            qDebug() << "AP_INFO:" << obj.toVariantMap();
        }else{
            qDebug() << "Wrong format of JIO_AP_INFO msg:(" << error.errorString() << ")\n";
        }
    }else {
        qDebug() << "TODO: Not support cmd: " << cmd;
    }
}

void DlgJIO::setTableWidgetBGColor(QTableWidget *tw, int row, int col, QColor color)
{
    QTableWidgetItem *itm = tw->item(row, col);
    if (itm){
        itm->setBackground(QBrush(color));
    }else{
        qDebug() << "QTableWidget:" << tw << " do not have item on (row,col)=(" << row << "," << col << ")";
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

void DlgJIO::onAddIperf(QString cfg)
{
    emit sigAddIperf(cfg);
}

void DlgJIO::doRequestExec(QString targetIP, QString idx, QString sCmd)
{
    WSClient *wsc= mWScs[targetIP];
    //ask remote create serialport and start tcp server on port
    QString sendstr = QString("%1:%2:%3").arg(CMD_REQUEST_EXEC,
                                              idx,
                                              sCmd);
    int rc= wsc->sendText(sendstr);
    if (rc<=0){
        qDebug() << "send cmd Fail: " << sendstr;
    }
}

void DlgJIO::onStartOptimiz()
{
    if (mOptThread){
        qDebug() << "Start Optimize";
        mOptThread->start();
    }
}

void DlgJIO::onStopOptimiz()
{
    //TODO: DlgJIO::onStopOptimiz()
    // if (mOptWorker){
    //     qDebug() <<"stop Optimize";
    //     // mOptWorker->setStop(true);
    //     // emit stopOptimiz(); // cause loop (with connect())
    // }
}

void DlgJIO::initHanwha()
{
    connect(ui->pbHanwha, &QPushButton::clicked, this, &DlgJIO::showHanwha);
    mHanwha = new Hanwha();
    mDlgHanwha = new DlgHanwha(m_cfg, mHanwha);
    // connect(mHanwha, &Hanwha::newBeamTableIDs, mDlgHanwha, &DlgHanwha::onNewHanwhaBeamTableIDs);
    connect(mHanwha, &Hanwha::updateBeamTableData, mDlgHanwha, &DlgHanwha::onUpdateHanwhaBeamTableData);
    connect(mHanwha, &Hanwha::updateBeamTypes, mDlgHanwha, &DlgHanwha::onUpdateBeamTypes);
    connect(mHanwha, &Hanwha::updateBeamTypeGroup, mDlgHanwha, &DlgHanwha::onUpdateBeamTypeGroup);
    connect(mHanwha, &Hanwha::updateRefFile, mDlgHanwha, &DlgHanwha::setRefFileName);
    connect(mDlgHanwha, &DlgHanwha::reffilechanged, mHanwha, QOverload<QString>::of(&Hanwha::initBeamData));
    connect(this, &DlgJIO::closeAll, mDlgHanwha, &DlgHanwha::close);

    QResource resHanwha(":/AIP/Hanwha.xlsx");
    QString filename = resHanwha.fileName();
    QFile Hanwhafile(":/AIP/Hanwha.xlsx");
    if (Hanwhafile.open(QIODevice::ReadOnly)) {
        // qDebug() << "Calling Hanwha initBeamData with QFile...";
        mHanwha->initBeamData(&Hanwhafile);// Pass the address of the QFile object
        Hanwhafile.close(); // Close the file after initBeamData is done
    } else {
        qDebug() << "Failed to open" << filename << "for reading:" << Hanwhafile.errorString();
    }
}

void DlgJIO::showHanwha(bool checked)
{
    Q_UNUSED(checked)
    if (mDlgHanwha){
        QString beamtype = mDlgHanwha->getHanwhaBeamType();
        mDlgHanwha->onHanwhaBeamTypeTextChanged(beamtype);
        mDlgHanwha->activateWindow();
        mDlgHanwha->show();
    }
}

void DlgJIO::initCyntec()
{
    connect(ui->pbCyntec, &QPushButton::clicked, this, &DlgJIO::showCyntec);
    mCyntec = new Cyntec();
    mDlgCyntec = new DlgCyntec(m_cfg, mCyntec);
    connect(mCyntec, &Cyntec::newBeamFactorIDs, mDlgCyntec, &DlgCyntec::onNewCyntecBeamFactorIDs);
    // connect(mCyntec, &Cyntec::newBeamTableIDs, mDlgCyntec, &DlgCyntec::onNewCyntecBeamTableIDs);
    connect(mCyntec, &Cyntec::updateBeamFactorData, mDlgCyntec, &DlgCyntec::onUpdateCynteBeamFactorData);
    connect(mCyntec, &Cyntec::updateBeamTableData, mDlgCyntec, &DlgCyntec::onUpdateCyntecBeamTableData);
    connect(mCyntec, &Cyntec::updateRefFile, mDlgCyntec, &DlgCyntec::setRefFileName);
    connect(mCyntec, &Cyntec::updateBeamTypes, mDlgCyntec, &DlgCyntec::onUpdateBeamTypes);
    connect(mCyntec, &Cyntec::updateBeamTypeGroup, mDlgCyntec, &DlgCyntec::onUpdateBeamTypeGroup);
    connect(mCyntec, &Cyntec::updateBeamFactorSupport, mDlgCyntec, &DlgCyntec::onUpdateBeamFactorSupport);
    connect(mDlgCyntec, &DlgCyntec::reffilechanged, mCyntec, QOverload<QString>::of(&Cyntec::initBeamData));
    connect(this, &DlgJIO::closeAll, mDlgCyntec, &DlgCyntec::close);

    QResource resCyntec(":/AIP/Cyntec.xlsx");
    QString filename = resCyntec.fileName();
    // mModuleType = AIP::ModuleType::Cyntec;
    QFile Cyntecfile(":/AIP/Cyntec.xlsx");
    if (Cyntecfile.open(QIODevice::ReadOnly)) {
        // qDebug() << "Calling Cyntec initBeamData with QFile...";
        mCyntec->initBeamData(&Cyntecfile);// Pass the address of the QFile object
        Cyntecfile.close(); // Close the file after initBeamData is done
    } else {
        qDebug() << "Failed to open" << filename << "for reading:" << Cyntecfile.errorString();
    }
}

void DlgJIO::showCyntec(bool checked)
{
    Q_UNUSED(checked);
    if (mDlgCyntec){
        QString beamtype = mDlgCyntec->getCyntecBeamType();
        mDlgCyntec->onCyntecBeamTypeTextChanged(beamtype);
        mDlgCyntec->activateWindow();
        mDlgCyntec->show();
    }
}

void DlgJIO::initCmds()
{
    QFile fJio(":/jio/jiocmd");
    if (fJio.open(QIODevice::ReadOnly)) {
        //basic commands
        QByteArray jsonData = fJio.readAll();
        fJio.close();

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "Failed to parse JSON:" << parseError.errorString();
            return; // Or handle the error appropriately
        }
        jiocmdObj = jsonDoc.object();
        // rootObject.keys()

    }else {
        qDebug() << "Failed to open " << fJio.fileName() << " for reading:" << fJio.errorString();
    }
    QFile fHanwha(":/jio/hanwha");
    if (fHanwha.open(QIODevice::ReadOnly)) {

    }else {
        qDebug() << "Failed to open " << fHanwha.fileName() << " for reading:" << fHanwha.errorString();
    }

}

void DlgJIO::initTableWidget()
{
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
                                                       0.0, 359, 2,
                                                       ui->tableWidget);
    ui->tableWidget->setItemDelegateForColumn(GPScols::Heading, dHeadDelegate);

    NumberDelegate *dPitchDelegate = new NumberDelegate(NumberDelegate::Double,
                                                        -90.0, 90, 2,
                                                        ui->tableWidget);
    ui->tableWidget->setItemDelegateForColumn(GPScols::Pitch, dPitchDelegate);

    //
    ui->twResult->setColumnWidth(AZEIcols::Name, 80);
    ui->twResult->setColumnWidth(AZEIcols::Distance, 90);
    ui->twResult->setColumnWidth(AZEIcols::P1Azimuth, 50);
    ui->twResult->setColumnWidth(AZEIcols::P2Azimuth, 50);
    ui->twResult->setColumnWidth(AZEIcols::P1Elevation, 50);
    ui->twResult->setColumnWidth(AZEIcols::P2Elevation, 50);
    ui->twResult->setColumnWidth(AZEIcols::P2AzDiff, 50);
    ui->twResult->setColumnWidth(AZEIcols::P2ElDiff, 50);
    ui->twResult->setColumnWidth(AZEIcols::BeamDirID, 80);
    // ui->twResult->setColumnWidth(AZEIcols::P2BFTx1Att, 65);
    // ui->twResult->setColumnWidth(AZEIcols::P2BFTx2Att, 65);
    // ui->twResult->setColumnWidth(AZEIcols::P2Tx1Att, 65);
    // ui->twResult->setColumnWidth(AZEIcols::P2Tx2Att, 65);
    // ui->twResult->setColumnWidth(AZEIcols::P2BFRx1Att, 65);
    // ui->twResult->setColumnWidth(AZEIcols::P2BFRx2Att, 65);
    // ui->twResult->setColumnWidth(AZEIcols::P2Rx1Att, 65);
    // ui->twResult->setColumnWidth(AZEIcols::P2Rx2Att, 65);
    // ui->twResult->setColumnWidth(AZEIcols::P2RxLnaAtt, 65);

    // Only accept Double
    NumberDelegate *dDelegate = new NumberDelegate(NumberDelegate::Double,
                                                   0.0, 10000.0, 2,
                                                   ui->twResult);
    ui->twResult->setItemDelegateForColumn(AZEIcols::Distance, dDelegate);
    NumberDelegate *dAziDelegate = new NumberDelegate(NumberDelegate::Double,
                                                      0.0, 360.0, 2,
                                                      ui->twResult);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P1Azimuth, dAziDelegate);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P2Azimuth, dAziDelegate);
    NumberDelegate *dElDelegate = new NumberDelegate(NumberDelegate::Double,
                                                     -90.0, 90.0, 2,
                                                     ui->twResult);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P1Elevation, dElDelegate);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P2Elevation, dElDelegate);
    NumberDelegate *dAttDelegate = new NumberDelegate(NumberDelegate::Double,
                                                     0.0, 120.0, 2,
                                                     ui->twResult);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P2BFTx1Att, dAttDelegate);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P2BFTx2Att, dAttDelegate);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P2Tx1Att, dAttDelegate);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P2Tx2Att, dAttDelegate);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P2BFRx1Att, dAttDelegate);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P2BFRx2Att, dAttDelegate);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P2Rx1Att, dAttDelegate);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P2Rx2Att, dAttDelegate);
    NumberDelegate *dLanAttDelegate = new NumberDelegate(NumberDelegate::Double,
                                                      -12.0, 0.0, 2,
                                                      ui->twResult);
    ui->twResult->setItemDelegateForColumn(AZEIcols::P2RxLnaAtt, dLanAttDelegate);

    ui->twAIP->setItemDelegateForColumn(AIPcols::Azimuth, dAziDelegate);
    ui->twAIP->setItemDelegateForColumn(AIPcols::Elevation, dElDelegate);

    ui->twAIP->setColumnWidth(AIPcols::Azimuth, 80);
    ui->twAIP->setColumnWidth(AIPcols::Elevation, 80);
    ui->twAIP->setColumnWidth(AIPcols::BeamDirectionID, 120);

    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested,
            this, &DlgJIO::showContextMenu);
    connect(ui->tableWidget, &QTableWidget::currentCellChanged,
            this, &DlgJIO::onDeviceCellChanged);
}

void DlgJIO::initAction()
{
    //tableWidget right menu
    m_contextMenu = new QMenu(this);
    m_insertAction = m_contextMenu->addAction("Insert");
    connect(m_insertAction, &QAction::triggered, this , &DlgJIO::onInsert);
    m_deleteAction = m_contextMenu->addAction("Delete");
    connect(m_deleteAction, &QAction::triggered, this , &DlgJIO::onDelete);
    m_contextMenu->addSeparator();
    // get GPS info
    m_GPSAction = m_contextMenu->addAction("Get GPS");
    connect(m_GPSAction, &QAction::triggered, this , &DlgJIO::onGetGPS);
    // get Sensor info
    m_SensorAction = m_contextMenu->addAction("Get Sensor");
    connect(m_SensorAction, &QAction::triggered, this , &DlgJIO::onGetSensor);


    m_clearAction = new QAction("clear");
        // m_contextMenu->addAction("clear");
    connect(m_clearAction, &QAction::triggered, this , &DlgJIO::onClear);

    connect(ui->pbLoad, &QPushButton::clicked, this, &DlgJIO::onLoadCliecked);
    connect(ui->pbSave, &QPushButton::clicked, this, &DlgJIO::onSaveCliecked);
    connect(ui->pbCalc, &QPushButton::clicked, this, &DlgJIO::onCalcCliecked);
    // connect(ui->pbShowMap, &QPushButton::clicked, this, &DlgJIO::onShowMap);
    connect(ui->pbShowGeo, &QPushButton::clicked, this, &DlgJIO::onShowGeo);
    connect(ui->pbShow3D, &QPushButton::clicked, this, &DlgJIO::onShow3D);
    connect(ui->pbClear, &QPushButton::clicked, m_clearAction, &QAction::triggered);
    connect(ui->pbToDMS, &QPushButton::clicked, this, &DlgJIO::onToDMS);
    connect(ui->pbToDegree, &QPushButton::clicked, this, &DlgJIO::onToDegree);
    connect(ui->pbSet, &QPushButton::clicked, this, &DlgJIO::onSet);
    // keep quire device
    connect(ui->pbInquire, &QPushButton::clicked, this, &DlgJIO::onInquireClicked);
    // Do Optimiz

    connect(ui->pbOptimize, &QPushButton::clicked, this, &DlgJIO::onOptimizeClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &DlgJIO::close); // close button click

    connect(this, &DlgJIO::TileAvailable, this , &DlgJIO::onTileAvailable);

    connect(ui->pbCMBeamDirIDInit, &QPushButton::clicked, this, &DlgJIO::onCMBeamDirIDInit);
    ui->pbAttInit->setVisible(false);
    connect(ui->pbAttInit, &QPushButton::clicked, this, &DlgJIO::onAttInit);
    connect(ui->pbBeamDirIDCmd, &QPushButton::clicked, this, &DlgJIO::onBeamDirIDCmd);
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
    QTableWidgetItem *itm = ui->tableWidget->item(iRow, GPScols::PositionName);
    QString name="";
    if (itm!=nullptr){
        name = itm->text();
    }
    // QString name =
    qDebug() << "onDelete:" <<  QString::number(iRow);
    ui->tableWidget->removeRow(iRow);
    if (!name.isEmpty()){
        emit deleteItm(name);
    }
}

void DlgJIO::onGetGPS(bool checked)
{
    Q_UNUSED(checked)
    QMap<int, QString> data;
    int row;
    QString ip;
    QModelIndexList ls= ui->tableWidget->selectionModel()->selectedIndexes();
    foreach (auto midx, ls){
        row = midx.row();
        if (!data.contains(row)){
            ip = ui->tableWidget->item(row, GPScols::IPAddr)->text();
            data.insert(row, ip);
        }
    }
    foreach (int key, data.keys()){
        if (!data.value(key).isEmpty()){
            getGpsInfo(QString::number(key), data.value(key));
        }
    }
}

void DlgJIO::onGetSensor(bool checked)
{
    Q_UNUSED(checked)
    QMap<int, QString> data;
    int row;
    QString ip;
    QModelIndexList ls= ui->tableWidget->selectionModel()->selectedIndexes();
    foreach (auto midx, ls){
        row = midx.row();
        if (!data.contains(row)){
            ip = ui->tableWidget->item(row, GPScols::IPAddr)->text();
            data.insert(row, ip);
        }
    }
    foreach (int key, data.keys()){
        if (!data.value(key).isEmpty()){
            getSensorInfo(QString::number(key), data.value(key));
        }
    }
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
    ui->tableWidget->setItem(iRow, GPScols::PositionName, new QTableWidgetItem(iconForState("init"), name));
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

void DlgJIO::initBeamCMD(QString c, QString antarraymode, QString cmName)
{
    QString cmd = mHanwha->getCmd("POWER_OFF").arg(c);
    if(cmName.isEmpty()){
        emit addBeamIDCmd(cmd);
    }else{
        emit addCMBeamIDCmd(cmName, cmd);
    }

    cmd =mHanwha->getCmd("POWER_ON").arg(c);
    if(cmName.isEmpty()){
        emit addBeamIDCmd(cmd);
    }else{
        emit addCMBeamIDCmd(cmName, cmd);
    }

    cmd = mHanwha->getCmd("INIT").arg(c);
    if(cmName.isEmpty()){
        emit addBeamIDCmd(cmd);
    }else{
        emit addCMBeamIDCmd(cmName, cmd);
    }

    cmd = mHanwha->getCmd("REG").arg(c);
    if(cmName.isEmpty()){
        emit addBeamIDCmd(cmd);
    }else{
        emit addCMBeamIDCmd(cmName, cmd);
    }

    double freq = ui->cbRFFreq->currentText().toDouble();
    QString fidx = mHanwha->getFreqIdx(freq);
    if (fidx != "") {
        cmd = mHanwha->getCmd("SET_Freq").arg(c, fidx);
        if(cmName.isEmpty()){
            emit addBeamIDCmd(cmd);
        }else{
            emit addCMBeamIDCmd(cmName, cmd);
        }
    }else {
        qDebug() << "Did not get freq id :" << freq;
    }

    cmd = mHanwha->getCmd("SET_AntArrayMode").arg(c, antarraymode);
    if(cmName.isEmpty()){
        emit addBeamIDCmd(cmd);
    }else{
        emit addCMBeamIDCmd(cmName, cmd);
    }
}

void DlgJIO::initBeamIdCMD(QString c, QString beamid, QString cmName)
{
    QString cmd="";
    if (beamid.contains("TODO")){
        cmd = beamid;
    }else {
        cmd = mHanwha->getCmd("SET_BeamID").arg(c, beamid);
    }
    if(cmName.isEmpty()){
        emit addBeamIDCmd(cmd);
    }else{
        emit addCMBeamIDCmd(cmName, cmd);
    }
}

void DlgJIO::initBeamTxAttCMD(QString c, QString bfTx1, QString bfTx2,
                              QString Tx1att, QString Tx2att, QString cmName)
{
    QString cmd = mHanwha->getCmd("SET_TxTotalAttn").arg(c, bfTx1, bfTx2);
    if(cmName.isEmpty()){
        emit addBeamIDCmd(cmd);
    }else{
        emit addCMBeamIDCmd(cmName, cmd);
    }

    cmd = mHanwha->getCmd("SET_TxAttn").arg(c, Tx1att, Tx2att);
    if(cmName.isEmpty()){
        emit addBeamIDCmd(cmd);
    }else{
        emit addCMBeamIDCmd(cmName, cmd);
    }
}

void DlgJIO::initBeamRxAttCMD(QString c, QString bfRx1, QString bfRx2,
                              QString Rx1att, QString Rx2att, QString RxLan,
                              QString cmName)
{
    QString cmd = mHanwha->getCmd("SET_RxTotalAttn").arg(c, bfRx1, bfRx2);
    if(cmName.isEmpty()){
        emit addBeamIDCmd(cmd);
    }else{
        emit addCMBeamIDCmd(cmName, cmd);
    }
    cmd = mHanwha->getCmd("SET_RxAttn").arg(c, Rx1att,Rx2att);
    if(cmName.isEmpty()){
        emit addBeamIDCmd(cmd);
    }else{
        emit addCMBeamIDCmd(cmName, cmd);
    }
    // Rx lan att
    cmd = mHanwha->getCmd("SET_LnaAttn").arg(c, RxLan);
    if(cmName.isEmpty()){
        emit addBeamIDCmd(cmd);
    }else{
        emit addCMBeamIDCmd(cmName, cmd);
    }
}

void DlgJIO::getBestBeamID(int idx,
                           double azimuthDegree,
                           AIP::ModuleType aiptype, QList<QTableWidgetItem*> cm7rs,
                           double maxDistance)
{
    // idx: 0: AIP1, 1:AIP2
    QList<double> aipDs;
    double minaz=360;
    double maxaz=0;
    double az;
    double aipaz=0;
    double aipazdiff=0;
    if (cm7rs.length()>1){
        for(auto azitm: cm7rs){
            az = azitm->text().toDouble();
            if (az>maxaz){
                maxaz = az;
            }
            if (az<minaz){
                minaz = az;
            }
            aipDs.append(az);
        }
        aipaz = averageBearing(aipDs);
    }else if (cm7rs.length()==1){
        aipaz = cm7rs.value(0)->text().toDouble();
    }else {
        qDebug() << "No AIP"<< idx << " AZ value";
    }
    aipazdiff = aipaz-azimuthDegree;
    //AIP1
    ui->twAIP->setItem(idx, AIPcols::Azimuth,
                       new QTableWidgetItem(QString::number(aipaz)));
    ui->twAIP->setItem(idx, AIPcols::Elevation,
                       new QTableWidgetItem(ui->leAM7el->text()));
    ui->twAIP->setItem(idx, AIPcols::Azdiff,
                       new QTableWidgetItem(QString::number(aipazdiff)));
    //Get best Cyntec/Hanwha AM7 id
    emit addBeamIDCmd("#AM7 AIP-"+ QString::number(idx));
    qDebug() << "AIP:" << idx << " Max az:" << maxaz << " Min Az:" << minaz;
    QVector<double> ds;
    QString bfTx1="";
    QString bfTx2="";
    QString Tx1="";
    QString Tx2="";
    QString bfRx1="0.0";
    QString bfRx2="0.0";
    QString Rx1="20.0";
    QString Rx2="20.0";
    QString RxLan="0";
    int beamid=-1;
    if (aiptype==AIP::ModuleType::Cyntec){
        //TODO: Cyntec getBestBeamID
        // ui->twAIP->setItem(idx, AIPcols::BeamDirectionID,
        //                    new QTableWidgetItem("99"));
    }else if (aiptype==AIP::ModuleType::Hanwha){
        QString c=""; //cmd name diff to AIP1/AIP2
        if (idx==0){
            //
        }else if (idx==1){
            c="2";
        }
        initBeamCMD(c);
        // Hanwha getBestBeamID
        beamid = mHanwha->getBestBeamID(minaz, maxaz , 0, 0);
        if (idx==1){
            beamid = beamid - 1;
        }
        initBeamIdCMD(c, QString::number(beamid));
        qDebug() << "// TODO: get Att value by distance (use The most remote CM's distance)" << maxDistance;

        ds = mHanwha->getBFTxAtt(maxDistance);
        if (ds.length()>=2){
            bfTx1 = QString::number(ds[0]);
            bfTx2 = QString::number(ds[1]);
        }
        // ds = mHanwha->getBFRxAtt(maxDistance);
        // if (ds.length()>=2){
        //     bfRx1 = QString::number(ds[0]);
        //     bfRx2 = QString::number(ds[1]);
        // }

        ds = mHanwha->getTxAtt(maxDistance);
        if (ds.length()>=2){
            Tx1 = QString::number(ds[0]);
            Tx2 = QString::number(ds[1]);
        }
        // ds = mHanwha->getRxAtt(maxDistance);
        // if (ds.length()>=3){
        //     Rx1 = QString::number(ds[0]);
        //     Rx2 = QString::number(ds[1]);
        //     RxLan = QString::number(ds[2]);

        // }
        if (maxDistance<=50){
            Rx1 = "30.0";
            Rx2 = "30.0";
            RxLan = "12";
        }
        // Tx att
        initBeamTxAttCMD(c, bfTx1, bfTx2, Tx1, Tx2);
        // Rx att
        initBeamRxAttCMD(c, bfRx1, bfRx2, Rx1, Rx2, RxLan);
        //
        emit addBeamIDCmd(mHanwha->getCmd("ATC_ON").arg(c));
        //
        emit addBeamIDCmd("--------------------------------------------------------------------------------");
        emit addBeamIDCmd(mHanwha->getCmd("GET_EIRP").arg(c, QString::number(1)));
        emit addBeamIDCmd(mHanwha->getCmd("GET_EIRP").arg(c, QString::number(2)));
        emit addBeamIDCmd("================================================================================");
    }else{
        qDebug() << "Unknown AIP" << idx << " type:" << aiptype;
    }
    ui->twAIP->setItem(idx, AIPcols::BeamDirectionID,
                       new QTableWidgetItem(QString::number(beamid)));
    ui->twAIP->setItem(idx, AIPcols::BFTx1Att, new QTableWidgetItem(bfTx1));
    ui->twAIP->setItem(idx, AIPcols::BFTx2Att, new QTableWidgetItem(bfTx2));
    ui->twAIP->setItem(idx, AIPcols::Tx1Att, new QTableWidgetItem(Tx1));
    ui->twAIP->setItem(idx, AIPcols::Tx2Att, new QTableWidgetItem(Tx2));

    ui->twAIP->setItem(idx, AIPcols::BFRx1Att, new QTableWidgetItem(bfRx1));
    ui->twAIP->setItem(idx, AIPcols::BFRx2Att, new QTableWidgetItem(bfRx2));
    ui->twAIP->setItem(idx, AIPcols::Rx1Att, new QTableWidgetItem(Rx1));
    ui->twAIP->setItem(idx, AIPcols::Rx2Att, new QTableWidgetItem(Rx2));
    ui->twAIP->setItem(idx, AIPcols::RxLnaAtt, new QTableWidgetItem(RxLan));
}

void DlgJIO::initResultHeader(AIP::ModuleType aip1type)
{
    QStringList hls;
    hls << mHeaderResult;
    if (aip1type == AIP::ModuleType::Hanwha){
        ui->twResult->setColumnCount(18);
        hls << mHeaderHanwha;
    }else if (aip1type == AIP::ModuleType::Cyntec){
        qDebug() << "Change twAIP column header name to fit Cyntec";
    }
    ui->twResult->setHorizontalHeaderLabels(hls);
}

void DlgJIO::onCalcCliecked(bool checked)
{
    Q_UNUSED(checked)
    emit clearBeamIDCmd();
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
    //AM7 head
    //AM7 Pitch
    //AM7 AIP1 type
    AIP::ModuleType aip1type = getModuleType(0, static_cast<int>(GPScols::AIP1));
    if (aip1type == AIP::ModuleType::Unknown){
        QString errmsg = "Please setup ModuleType of AIP1";
        QMessageBox::warning(this, "Error", errmsg, QMessageBox::Ok);
        ui->tableWidget->selectRow(0);
        return;
    }
    //AM7 AIP2
    AIP::ModuleType aip2type = getModuleType(0, static_cast<int>(GPScols::AIP2));
    if (aip2type == AIP::ModuleType::Unknown){
        QString errmsg = "Please setup ModuleType of AIP2";
        QMessageBox::warning(this, "Error", errmsg, QMessageBox::Ok);
        ui->tableWidget->selectRow(0);
        return;
    }

    // double msl1 = GeoTranslate::convertEllipsoidToMSL(lat1, lon1, alt1);
    // qDebug() << " Pos:" << pos1 << " Elevation hight:" << QString::number(msl1);

    initResultHeader(aip1type);

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
    //calc distance & azimuth & Elevation
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
        ui->twResult->setItem(i-1, AZEIcols::P1Azimuth, new QTableWidgetItem(QString::number(azimuth, 'f', 1)));
        ui->twResult->setItem(i-1, AZEIcols::P2Azimuth, new QTableWidgetItem(QString::number(azimuth2, 'f', 1)));

        // totalazimuth = totalazimuth + azimuth;
        el1 = GeoTranslate::calcElevationAngle(altmsl1, altmsl, distance*1000);
        el2 = GeoTranslate::calcElevationAngle(altmsl, altmsl1, distance*1000);
        // qDebug() << " " << QString::number(altmsl1) << " - "  << QString::number(altmsl)
        //          << " distance:" << QString::number(distance)
        //          << " el1:" << QString::number(el1) << " el2:" << QString::number(el2);
        ui->twResult->setItem(i-1, AZEIcols::P1Elevation, new QTableWidgetItem(QString::number(el1, 'f', 1)));
        ui->twResult->setItem(i-1, AZEIcols::P2Elevation, new QTableWidgetItem(QString::number(el2, 'f', 1)));
        // elbearings.append(el1);
        totalel = totalel + el1;
    }
    // AM7 heading
    double azimuthDegree = averageBearing(azbearings);
    // qDebug() << " azimuthDegree:" << QString::number(azimuthDegree);
    ui->leAM7az->setText(QString::number(azimuthDegree, 'f', 1));
    // AM7 Pitch
    double elDegree = totalel/ui->twResult->rowCount();
    ui->leAM7el->setText(QString::number(elDegree, 'f', 1));

    //group CM7 by Azimuth2
    QColor lColor = QColor(144, 238, 144); //light green
    QColor rColor = QColor(173, 216, 230); //light blue
    QColor nColor = QColor(Qt::lightGray);
    double relative;
    double cmaz;
    double distMaxR=0.0;
    double distR=0.0;
    double distMaxL=0.0;
    double distL=0.0;
    QList<QTableWidgetItem*> cm7rs;
    QList<QTableWidgetItem*> cm7ls;
    for (int iRow=0;iRow<ui->twResult->rowCount();iRow++){
        QTableWidgetItem *ditm = ui->twResult->item(iRow, AZEIcols::Distance);
        QTableWidgetItem *itm = ui->twResult->item(iRow, AZEIcols::P1Azimuth);
        if (itm){
            cmaz = itm->text().toDouble();// + 180;
            relative = fmod((cmaz - azimuthDegree + 360), 360);
            // qDebug() << "cmaz:" << cmaz << "  relative:" << relative;
            if (relative > 0 && relative < 90){
                //azimuthDegree 的第一象限
                itm->setBackground(QBrush(lColor));
                itm->setToolTip("CM7-FirstQuadrant");
                itm->setData(Qt::UserRole, "AIP1");
                cm7rs.append(itm);
                distR =ditm->text().toDouble()*1000;
                if (distR>distMaxR){
                    distMaxR = distR;
                }
            }else if (relative > 270 && relative < 360){
                //azimuthDegree 的第四象限
                itm->setBackground(QBrush(rColor));
                itm->setToolTip("CM7-FourthQuadrant");
                itm->setData(Qt::UserRole, "AIP2");
                cm7ls.append(itm);
                distL =ditm->text().toDouble()*1000;
                if (distL>distMaxL){
                    distMaxL = distL;
                }
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
    // ui->tableWidget->item(0, GPScols::AIP1); //cyntec or hanwha
    QStringList hls;
    hls << mHeaderAIP;
    if (aip1type == AIP::ModuleType::Hanwha){
        ui->twAIP->setColumnCount(13);
        hls << mHeaderHanwha;
    }else if (aip1type == AIP::ModuleType::Cyntec){
        qDebug() << "Change twAIP column header name to fit Cyntec";
    }
    ui->twAIP->setHorizontalHeaderLabels(hls);
    //AM7 AIP1 Az, TODO El
    getBestBeamID(0, azimuthDegree, aip1type, cm7rs, distMaxR);
    //AM7 AIP2
    getBestBeamID(1, azimuthDegree, aip2type, cm7ls, distMaxL);

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

        qDebug() << "Inquire start after 3 sec";
        m_InquireTimer->start(3000);// 3sec

    }else{
        if (m_InquireTimer->isActive()){
            m_InquireTimer->stop();
        }
        for(int row=0;row < ui->tableWidget->rowCount(); row ++){
            setStateIcon(row, GPScols::PositionName, "init");
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }
}

void DlgJIO::onOptimizeClicked(bool checked)
{
    if (checked){
        qDebug() <<"//do Optimiz to get All device's Beam Direction ID/ Att value";
        // init data
        QJsonObject dataobj = createInitData();
        // Optimiz worker run in thread
        // worker
        mOptWorker = new OptimizeWorker(dataobj);
        connect(mOptWorker, &OptimizeWorker::started, this, &DlgJIO::onOptimizeStarted);
        connect(mOptWorker, &OptimizeWorker::debugMsg, this, &DlgJIO::onOptimizeWorkerDebug);
        connect(mOptWorker, &OptimizeWorker::sigAddIperf, this, &DlgJIO::onAddIperf);
        connect(this, &DlgJIO::stopOptimiz, mOptWorker, &OptimizeWorker::Stop);
        // thread
        mOptThread = new QThread();
        connect(mOptThread, &QThread::started, mOptWorker, &OptimizeWorker::work);
        // connect(mOptWorker, &OptimizeWorker::stoped, mOptThread, &QThread::deleteLater);
        connect(mOptWorker, &OptimizeWorker::stoped, this, &DlgJIO::onOptimizeStoped);

        mOptWorker->moveToThread(mOptThread);

        mDlgOptimize->show();
        mDlgOptimize->raise();           // Bring to top of Z-order
        mDlgOptimize->activateWindow();  // Request focus
        emit startOptimiz();
    } else {
        // TODO: check run status?
        emit stopOptimiz();
    }
}

void DlgJIO::onInquireTimerTimeout()
{
    if (ui->tableWidget->rowCount()>0){
        qDebug() << "do Inquire";
        WSClient *client;
        for (int row=0; row < ui->tableWidget->rowCount(); row++){
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            //IPAddr
            QTableWidgetItem *itm= ui->tableWidget->item(row, GPScols::IPAddr);
            if (itm){
                QString target = itm->text();
                if (!target.isEmpty()){
                    if (!mWScs.contains(target)){
                        QString s = "ws://"+target+":"+QString::number(QIPERFD_WSPORT);
                        client = new WSClient(target, QUrl(s), "", true);
                        connect(client, &WSClient::connected, this, &DlgJIO::onConnected);
                        connect(client, &WSClient::disconnected, this, &DlgJIO::onDisconnected);
                        connect(client, &WSClient::requestResult, this, &DlgJIO::onRequestResult);
                        mWScs[target] = client;
                    }else{
                        client = mWScs[target];
                    }
                    if (client->isConnected()){
                        // qDebug() << "onInquireTimerTimeout //TODO Inquire:" << target;
                        getGpsInfo(QString::number(row), target);
                        getSensorInfo(QString::number(row), target);
                    }else{
                        qDebug() << "onInquireTimerTimeout: WSClient not connect, ICON NG";
                        setStateIcon(row, GPScols::PositionName, "NG");
                    }

                }else{
                    qDebug() << "No IPAddr at row:" << row << ", col:" << static_cast<int>(GPScols::IPAddr);
                    setTableWidgetBGColor(ui->tableWidget,
                                          row, static_cast<int>(GPScols::IPAddr), QColor("Red"));
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

void DlgJIO::onDisconnected(QString from)
{
    if (mWScs.contains(from)){
        mWScs.remove(from);
    }
    updateStats(from, "NG");
}

void DlgJIO::onConnected(QString from)
{
    updateStats(from, "OK");
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

    //TODO: Do not do Manual calc
    // if (ui->tableWidget->rowCount()>1){
    //     onCalcCliecked(true);
    // }

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

void DlgJIO::onCMBeamDirIDInit(bool checked)
{   Q_UNUSED(checked)
    //calc all CM7 beam Direction ID by Az/El diff
    if (ui->twResult->rowCount()>0){
        emit clearCMBeamIDCmd();
        QString cm="";
        //TODO: AIP type
        AIP::ModuleType aiptype = AIP::ModuleType::Unknown;
        // QString aipn;
        // QTableWidgetItem *aipitem;
        int initID=-1;
        double realHeading=0.0;
        double realPitch=0.0;
        double expectHeading=0.0;
        double expectPitch=0.0;
        double diffHead=0.0;
        double diffPitch=0.0;
        for (int iRow=0;iRow<ui->twResult->rowCount();iRow++){
            cm = ui->tableWidget->item(iRow+1, GPScols::PositionName)->text();

            realHeading = ui->tableWidget->item(iRow+1, GPScols::Heading)->text().toDouble();
            realPitch = ui->tableWidget->item(iRow+1, GPScols::Pitch)->text().toDouble();
            aiptype = getModuleType(iRow+1 ,GPScols::AIP1);
            if (aiptype == AIP::ModuleType::Unknown){
                QString errmsg = "Please setup ModuleType";
                QMessageBox::warning(this, "Error", errmsg, QMessageBox::Ok);
                ui->tableWidget->selectRow(iRow+1);
                break;
            }

            expectHeading = ui->twResult->item(iRow, AZEIcols::P2Azimuth)->text().toDouble();
            expectPitch = ui->twResult->item(iRow, AZEIcols::P2Elevation)->text().toDouble();
            // qDebug() << cm << " heading real:" << QString::number(realHeading)
            //          << " ,Expect:" << QString::number(expectHeading)
            //          << " El read:" << QString::number(realPitch)
            //          << " ,Expect:" << QString::number(expectPitch);
            diffHead = expectHeading - realHeading;
            ui->twResult->setItem(iRow, AZEIcols::P2AzDiff,
                                  new QTableWidgetItem(QString::number(diffHead)));
            diffPitch = expectPitch - realPitch;
            ui->twResult->setItem(iRow, AZEIcols::P2ElDiff,
                                  new QTableWidgetItem(QString::number(diffPitch)));
            //get beam Direction ID
            emit addCMBeamIDCmd(cm, "# "+cm);
            initBeamCMD("", "8x8", cm);
            initID = getNearestBeamDirectionID(cm, aiptype, diffHead, diffPitch);
            QTableWidgetItem *itm = new QTableWidgetItem(QString::number(initID));
            if (initID<0){
                itm->setBackground(QBrush(Qt::red));
                initBeamIdCMD("", "## TODO: Did not get init Beam Direction ID", cm);
            }else{
                initBeamIdCMD("", QString::number(initID), cm);
            }
            ui->twResult->setItem(iRow, AZEIcols::BeamDirID, itm);

            //Use ID's deg+ phy deg draw arrow
        }
        onAttInit();
    }

}

void DlgJIO::onAttInit(bool checked)
{
    Q_UNUSED(checked)
    QString cm="";
    AIP::ModuleType aiptype = AIP::ModuleType::Unknown;
    double freq= ui->cbRFFreq->currentText().toDouble() * 1000000000;
    double distance=0.0;
    double fspl=0.0;
    double targetEIRP=0;
    QVector<double> ds;
    for (int iRow=0;iRow<ui->twResult->rowCount();iRow++){
        cm = ui->tableWidget->item(iRow+1, GPScols::PositionName)->text();
        distance = ui->twResult->item(iRow, AZEIcols::Distance)->text().toDouble()*1000;
        fspl = MyFunc::calculateFSPL(distance, freq);
        qDebug() << iRow << "freq:" << freq << " ,distance:" << distance <<" fspl:" << fspl;
        aiptype = getModuleType(iRow+1 ,GPScols::AIP1);
        if (aiptype==AIP::ModuleType::Cyntec) {
            if (mCyntec){
                targetEIRP = mCyntec->getTargetEIRP(distance);
                qDebug() <<"mCyntec: " << iRow << " targetEIRP:" << targetEIRP;
                qDebug() <<"TODO mCyntec  TxAtt";
                ds = mCyntec->getRxAtt(distance);
                if (ds.length()>=2){
                    ui->twResult->setItem(iRow, AZEIcols::P2Rx1Att,
                                          new QTableWidgetItem(QString::number(ds[0])));
                    ui->twResult->setItem(iRow, AZEIcols::P2Rx2Att,
                                          new QTableWidgetItem(QString::number(ds[1])));
                }
                ds = mCyntec->getBFAtt(distance);
                if (ds.length()>=2){
                    ui->twResult->setItem(iRow, AZEIcols::P2BFRx1Att,
                                          new QTableWidgetItem(QString::number(ds[0])));
                    ui->twResult->setItem(iRow, AZEIcols::P2BFRx2Att,
                                          new QTableWidgetItem(QString::number(ds[1])));
                }

            }
        }else if (aiptype==AIP::ModuleType::Hanwha) {
            qDebug() << "[onAttInit] Hanwha";
            if (mHanwha){
                targetEIRP = mHanwha->getTargetEIRP(distance);
                qDebug() << iRow << " targetEIRP:" << targetEIRP;
                QString bfTx1="";
                QString bfTx2="";
                QString Tx1="";
                QString Tx2="";
                QString bfRx1="";
                QString bfRx2="";
                QString Rx1="";
                QString Rx2="";
                QString RxLan="";
                ds = mHanwha->getBFTxAtt(distance);
                if (ds.length()>=2){
                    bfTx1 = QString::number(ds[0]);
                    bfTx2 = QString::number(ds[1]);
                    ui->twResult->setItem(iRow, AZEIcols::P2BFTx1Att, new QTableWidgetItem(bfTx1));
                    ui->twResult->setItem(iRow, AZEIcols::P2BFTx2Att, new QTableWidgetItem(bfTx2));
                }
                ds = mHanwha->getBFRxAtt(distance);
                if (ds.length()>=2){
                    bfRx1 = QString::number(ds[0]);
                    bfRx2 = QString::number(ds[1]);
                    ui->twResult->setItem(iRow, AZEIcols::P2BFRx1Att, new QTableWidgetItem(bfRx1));
                    ui->twResult->setItem(iRow, AZEIcols::P2BFRx2Att, new QTableWidgetItem(bfRx2));
                }
                ds = mHanwha->getTxAtt(distance);
                if (ds.length()>=2){
                    Tx1 = QString::number(ds[0]);
                    Tx2 = QString::number(ds[1]);
                    ui->twResult->setItem(iRow, AZEIcols::P2Tx1Att, new QTableWidgetItem(Tx1));
                    ui->twResult->setItem(iRow, AZEIcols::P2Tx2Att, new QTableWidgetItem(Tx2));
                }
                ds = mHanwha->getRxAtt(distance);
                if (ds.length()>=3){
                    Rx1 = QString::number(ds[0]);
                    Rx2 = QString::number(ds[1]);
                    ui->twResult->setItem(iRow, AZEIcols::P2Rx1Att, new QTableWidgetItem(Rx1));
                    ui->twResult->setItem(iRow, AZEIcols::P2Rx2Att, new QTableWidgetItem(Rx2));
                    RxLan = QString::number(ds[2]);
                    ui->twResult->setItem(iRow, AZEIcols::P2RxLnaAtt, new QTableWidgetItem(RxLan));
                }

                initBeamTxAttCMD("", bfTx1, bfTx2, Tx1, Tx2, cm);
                initBeamRxAttCMD("", bfRx1, bfRx2, Rx1, Rx2, RxLan, cm);
                emit addCMBeamIDCmd(cm, mHanwha->getCmd("ATC_ON").arg(""));
                emit addCMBeamIDCmd(cm, "----------------------------------------------------------------------");
            }
        }else{
            QString errmsg = "[onAttInit]Please setup ModuleType";
            QMessageBox::warning(this, "Error", errmsg, QMessageBox::Ok);
            ui->tableWidget->selectRow(iRow+1);
            break;
        }

    }
}

void DlgJIO::onBeamDirIDCmd(bool checked)
{
    Q_UNUSED(checked)
    if (mDlgBeamCmd){
        mDlgBeamCmd->activateWindow();
        // mDlgBeamCmd->exec();
        mDlgBeamCmd->show();
    }
}

void DlgJIO::showContextMenu(const QPoint &pos)
{
    if (ui->tableWidget->rowCount()<1){
        m_deleteAction->setEnabled(false);
        m_deleteAction->setVisible(false);
        m_GPSAction->setVisible(false);
        m_SensorAction->setVisible(false);
    }else {
        m_deleteAction->setEnabled(true);
        m_deleteAction->setVisible(true);
        if (ui->tableWidget->selectionModel()->selectedIndexes().count()>0){
            m_GPSAction->setVisible(true);
            m_SensorAction->setVisible(true);
        }else{
            m_GPSAction->setVisible(false);
            m_SensorAction->setVisible(false);
        }
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
            QTableWidgetItem *itmId=nullptr;
            QString label;
            AIP::ModuleType aiptype = AIP::ModuleType::Unknown;
            double lat0;
            double lon0;
            double lat;
            double lon;
            double heading;
            double azdeg;
            QRgb rgb1 = 0xFF6C6CF1; //Warm Blue
            QRgb rgbaz = 0xFF6CBCF1; //中等亮度的藏青色
            QGV::GeoPos am7;
            QGV::GeoPos cm;
            // marker & phy arrow line
            for(int row=0;row<ui->tableWidget->rowCount();row++){
                label = ui->tableWidget->item(row, GPScols::PositionName)->text();
                lat = ui->tableWidget->item(row, GPScols::Latitude)->text().toDouble();
                lon = ui->tableWidget->item(row, GPScols::Longitude)->text().toDouble();
                heading = ui->tableWidget->item(row, GPScols::Heading)->text().toDouble();
                aiptype = getModuleType(row ,GPScols::AIP1);
                if (row==0){
                    //AM7
                    lat0 = lat;
                    lon0 = lon;
                    am7 = QGV::GeoPos{lat0, lon0};
                    m_dlgGeo->addRectangle(am7, QPointF(20.0, 10.0), Qt::red, label);
                    //draw Main Arrow line
                    if (!ui->leAM7az->text().isEmpty()){
                        // expects
                        azdeg = ui->leAM7az->text().toDouble();
                        //TODO: length should not over range
                        m_dlgGeo->addArrowLine(am7, azdeg, 20, QColor(Qt::red), false, 1, 5);
                    }else{
                        //draw init Arrow Line
                        //TODO: length should not over range
                        m_dlgGeo->addArrowLine(am7, heading,
                                               30, QColor(Qt::blue), true, 2, 5);
                    }
                }else{
                    //CM7
                    // marker
                    cm = QGV::GeoPos{lat, lon};
                    //m_dlgGeo->addMarker(lat, lon, label);
                    m_dlgGeo->addRectangle(cm, QPointF(20.0, 10.0), Qt::yellow, label);

                    if (ui->twResult->rowCount()>0){
                        // Link Lines
                        m_dlgGeo->addLinkline(am7, cm, Qt::yellow, 2);
                        itm = ui->twResult->item(row-1, AZEIcols::P2Azimuth);
                        if (itm){
                            //draw Arrow line, expect
                            azdeg = itm->text().toDouble();
                            //TODO: length should not over range
                            m_dlgGeo->addArrowLine(cm, azdeg, 20, QColor(Qt::red), false, 1, 5);
                        }
                        // add later will be on top
                        itmId = ui->twResult->item(row-1, AZEIcols::BeamDirID);
                        if (itmId){
                            qDebug() << "have BeamDirID data:" << itmId->text();
                            azdeg = heading + getAz(aiptype,itmId->text().toInt());
                            //Beam Direction ID's az deg: (TODO:length should not over range)
                            m_dlgGeo->addArrowLine(cm, azdeg, 12, QColor(rgb1), false, 1, 3);
                        }


                    }else{
                        //draw init Arrow Line
                        //TODO: length should not over range
                        m_dlgGeo->addArrowLine(cm, heading,
                                               30, QColor(Qt::blue), true, 2, 5);
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
                            //TODO: length should not over range
                            m_dlgGeo->addArrowLine(am7, azdeg, 15,
                                                   QColor(rgbaz), false, 1, 5);
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
    if (!item){
        item = new QTableWidgetItem();
    }
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

void DlgJIO::onOptimizeStarted()
{
    ui->pbOptimize->setText("Stop");
}

void DlgJIO::onOptimizeStoped(int error)
{
    qDebug() << "onOptimizeStoped: error:" << error;
    ui->pbOptimize->setText("Optimiz");

    if (mOptThread){
        // mOptThread->deleteLater();
        // mOptThread->stop();
    }
}

void DlgJIO::onOptimizeWorkerDebug(QString msg)
{
    debug("OptimizeWorker:"+ msg);
}

QIcon DlgJIO::iconForState(const QString &state)
{
    if (state == "init")
        return QIcon(":/jio/INIT.png");
    else if (state == "OK")
        return QIcon(":/jio/OK.png");
    else if (state == "NG")
        return QIcon(":/jio/NG.png");
    else
        return QIcon();  // fallback
}

void DlgJIO::updateStats(QString target, QString state)
{
    for (int row=0; row< ui->tableWidget->rowCount(); row++){
        auto itm = ui->tableWidget->item(row, GPScols::IPAddr);
        if (itm){
            if (itm->text() == target){
                setStateIcon(row, GPScols::PositionName, state);
                break;
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

void DlgJIO::setStateIcon(int row, int column, QString state)
{
    QTableWidgetItem *item = ui->tableWidget->item(row, column);
    if (!item) {
        item = new QTableWidgetItem();
        ui->tableWidget->setItem(row, column, item);
    }

    item->setIcon(iconForState(state));
    item->setData(Qt::UserRole, state);  // Store state for later use
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
    // accroading IP address to get Location
    // when ready it will store at mIpLocation
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
        clearData();

        QJsonObject rootObject = jsonDoc.object();
        QString localaddr= rootObject.value("LocalAddr").toString();
        ui->leLocalAddr->setText(localaddr);
        mControlBy = static_cast<DlgSet::ControlBy>(rootObject.value("ControlBy").toInt());
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
    QJsonObject rootObject = createInitData();
    jsonDoc.setObject(rootObject);
    QString strJson(jsonDoc.toJson(QJsonDocument::Indented));
    QTextStream out(&file);
    out << strJson;
    out.flush();

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

int DlgJIO::getNearestBeamDirectionID(QString name, AIP::ModuleType aiptype, double diffHead, double diffPitch)
{
    // qDebug() << "getNearestBeamDirectionID:" << name
    //          << " az diff:" << diffHead
    //          << " el diff:" << diffPitch;
    int id=-1;
    if (aiptype==AIP::ModuleType::Cyntec){
        id = mCyntec->findClosestBeamID(diffHead, diffPitch);
    }else if (aiptype==AIP::ModuleType::Hanwha){
        id  = mHanwha->findClosestBeamID(diffHead, diffPitch);
    }else {
        qDebug() << "Unknown AIP type of " << name;
        return -1;
    }
    return id;
}

double DlgJIO::getAz(AIP::ModuleType aiptype, int BeamID)
{
    if (aiptype==AIP::ModuleType::Cyntec){
        return mCyntec->getAz(BeamID);
    }else if (aiptype==AIP::ModuleType::Hanwha){
        qDebug() << "getAz Hanwha";
        return 0.0;
    }else {
        qDebug() << "Unknown AIP type of " << aiptype;
        return 0.0;
    }
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
