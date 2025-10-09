#include "cyntec.h"

#include <QFile>
#include <QCoreApplication>
#include <QEventLoop>
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxworkbook.h"
using namespace QXlsx;

#include <QJsonParseError>
#include <QJsonDocument>

#include "myfunc.h"

#include <QDebug>

Cyntec::Cyntec(QObject *parent)
    : AIP{parent}
{
    mBeamFactorData = new QMap<int, CyntecBeamFactorData>();
    mBeamTableData = new QMap<int, CyntecBeamTableData>();
    mRangeDataObj = QJsonObject();
    initCmds();
}

void Cyntec::initCmds()
{
    QFile fCyntec(":/jio/cyntec");
    if (fCyntec.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = fCyntec.readAll();
        fCyntec.close();

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "Failed to parse JSON:" << parseError.errorString();
            return; // Or handle the error appropriately
        }
        cmdObj = jsonDoc.object();
        //TODO RESPONSE
        cmdObj.value("RESPONSE").toObject();
        // qDebug() << "cmdObj:" << cmdObj;

        mRangeDataObj = cmdObj.value("DistanceData").toObject();
        // qDebug() << "mRangeDataObj:" << mRangeDataObj;
    }else {
        qDebug() << "Failed to open " << fCyntec.fileName() << " for reading:" << fCyntec.errorString();
    }
}

void Cyntec::initBeamData(QString filename)
{
    QFile readFile(filename);
    if (!readFile.exists()){
        qDebug() << "File not exist: " << filename;
        return;
    }
    if (readFile.open(QIODevice::ReadOnly)) {
        qDebug() << "\nCalling initBeamData with QFile...";
        initBeamData(&readFile); // Pass the address of the QFile object
        readFile.close(); // Close the file after initBeamData is done
    } else {
        qDebug() << "Failed to open" << filename << "for reading:" << readFile.errorString();
    }

}

void Cyntec::initBeamData(QIODevice *filedevice)
{
    if (!filedevice) {
        qDebug() << "Error: QIODevice pointer is null.";
        return;
    }
    if (!filedevice->isOpen()) {
        qDebug() << "Error: QIODevice is not open.";
        return;
    }
    if (!filedevice->isReadable()) { // Or isWritable(), depending on intent
        qDebug() << "Error: QIODevice is not readable.";
        return;
    }
    QFile* file = qobject_cast<QFile*>(filedevice);
    emit updateRefFile(file->fileName());

    QXlsx::Document xlsReader(filedevice);
    if(xlsReader.load()){
        // std::shared_ptr<QXlsx::Cell> sharedCell;
        // Cell* cellB, cellC, cellD, cellE, cellF;
        QVariant varBeamID, varA, varAz, varEl, varHPAz, varHPEl, varBeamType;
        int irow=0;
        int icol=0;
        auto cell = xlsReader.cellAt(irow, icol);

        //BeamTable
        if (xlsReader.selectSheet("BeamTable")){
            mBeamTableData->clear();
            irow=2;
            icol=2;
            cell = xlsReader.cellAt(irow, icol);
            if (cell != NULL){
                varBeamID = cell->readValue();
                while (varBeamID.isValid()){
                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                    irow++;
                    cell = xlsReader.cellAt(irow, icol);
                    if (cell != NULL){
                        varBeamID = cell->readValue();  //BeamTableID
                        if(varBeamID.isValid()){
                            cell = xlsReader.cellAt(irow, 1);
                            if (cell != NULL){
                                varA = cell->readValue(); //support BeamFactorID list
                                foreach (QString id, varA.toString().split(",")){
                                    if (!mBeamFactorSupport.contains(id)){
                                        mBeamFactorSupport[id]=QList<int>();
                                    }
                                    mBeamFactorSupport[id].append(varBeamID.toInt());
                                }
                            }
                            cell = xlsReader.cellAt(irow, 3);
                            if (cell != NULL){
                                varAz = cell->readValue(); //AZ
                            }
                            cell = xlsReader.cellAt(irow, 4);
                            if (cell != NULL){
                                varEl = cell->readValue(); //EL
                            }
                            cell = xlsReader.cellAt(irow, 5);
                            if (cell != NULL){
                                varHPAz = cell->readValue(); //Azimuth 3dB BW (°)
                            }
                            cell = xlsReader.cellAt(irow, 6);
                            if (cell != NULL){
                                varHPEl = cell->readValue(); //Elevation 3dB BW (°)
                            }
                            cell = xlsReader.cellAt(irow, 7);
                            if (cell != NULL){
                                varBeamType = cell->readValue(); //comment
                                if (!mBeamTypeData.contains(varBeamType.toString())){
                                    mBeamTypeData[varBeamType.toString()]=QList<int>();
                                }
                                mBeamTypeData[varBeamType.toString()].append(varBeamID.toInt());
                                BeamTypeRange r = mBeamTypeRangeData.value(varBeamType.toString(), {0, 0, 0 ,0});
                                if (varAz.toDouble() < r.minAz){
                                    r.minAz = varAz.toDouble();
                                }
                                if (varAz.toDouble() > r.maxAz){
                                    r.maxAz = varAz.toDouble();
                                }
                                if (varEl.toDouble() < r.minEl){
                                    r.minEl = varEl.toDouble();
                                }
                                if (varEl.toDouble() > r.maxEl){
                                    r.maxEl = varEl.toDouble();
                                }
                                mBeamTypeRangeData[varBeamType.toString()] = r;
                            }
                            // qDebug() << "[TableData]varB:" << varB
                            //          << "varC:" << varC << " varD:" << varD
                            //          << "varE:" << varE << " varF:" << varF;
                            mBeamTableData->insert(varBeamID.toInt(),
                                                   CyntecBeamTableData(varBeamID.toInt(),
                                                                       varAz.toDouble(),
                                                                       varEl.toDouble(),
                                                                       varHPAz.toDouble(),
                                                                       varHPEl.toDouble(),
                                                                       varBeamType.toString()));
                        }else{
                            qDebug() << "BeamTable:No value row:" << irow << " col:" << icol;
                        }
                    }else{
                        // qDebug() << "[TableData]cell is NULL " << irow << "," << icol;
                        break;
                    }
                }
                // qDebug() << "mBeamTableData.keys:" << mBeamTableData->keys();
                QStringList tablekeys;
                for (int tablekey : mBeamTableData->keys()) {
                    // Convert the integer to a QString and add it to stringList
                    tablekeys.append(QString::number(tablekey));
                }
                // if (tablekeys.length()>0){
                //     emit newBeamTableIDs(tablekeys);
                // }
                if (mBeamTypeData.keys().count()>0){
                    emit updateBeamTypes(mBeamTypeData.keys());
                    emit updateBeamTypeGroup(mBeamTypeData);
                }
                if (mBeamFactorSupport.keys().count()>0){
                    emit updateBeamFactorSupport(mBeamFactorSupport);
                }
            } else {
                qDebug() << "sheet 'BeamTable' cell of (" << irow << "," << icol << ") is NULL";
            }
        }else{
            qDebug() << "sheet 'BeamTable' not found";
        }
        //BeamFactor
        if (xlsReader.selectSheet("BeamFactor")){
            mBeamFactorData->clear();
            irow=1;
            icol=2;
            cell = xlsReader.cellAt(irow, icol);
            if (cell != NULL){
                varBeamID = cell->readValue();
                // qDebug() << "varB: " << varB;
                while (varBeamID.isValid()){
                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                    irow++;
                    cell = xlsReader.cellAt(irow, icol);
                    if (cell != NULL){
                        varBeamID = cell->readValue();
                        cell = xlsReader.cellAt(irow, 3);
                        if (cell != NULL){
                            varAz = cell->readValue(); // Element Map
                        }
                        cell = xlsReader.cellAt(irow, 4);
                        if (cell != NULL){
                            varEl = cell->readValue(); // Att (dB)
                        }
                        cell = xlsReader.cellAt(irow, 5);
                        if (cell != NULL){
                            varHPAz = cell->readValue(); // Azimuth 3dB BW (°)
                        }
                        cell = xlsReader.cellAt(irow, 6);
                        if (cell != NULL){
                            varHPEl = cell->readValue(); // Elevation 3dB BW (°)
                        }
                        // qDebug() << "[FactorData]varB:" << varB
                        //          << "varC:" << varC << " varD:" << varD
                        //          << "varE:" << varE << " varF:" << varF;
                        mBeamFactorData->insert(varBeamID.toInt(),
                                                CyntecBeamFactorData(varBeamID.toInt(),
                                                                     varAz.toString(),
                                                                     varEl.toInt(),
                                                                     varHPAz.toDouble(),
                                                                     varHPEl.toDouble()));
                    }else{
                        // qDebug() << "[FactorData]cell is NULL " << irow << "," << icol;
                        break;
                    }
                }
                // qDebug() << "mBeamFactorData.keys:" << mBeamFactorData->keys().length();
                QStringList factorkeys;
                for (int factorkey : mBeamFactorData->keys()) {
                    // Convert the integer to a QString and add it to stringList
                    factorkeys.append(QString::number(factorkey));
                }
                if (factorkeys.length()>0){
                    // qDebug() << "factorkeys:" << factorkeys.join(",");
                    emit newBeamFactorIDs(factorkeys);
                }
            } else {
                qDebug() << "sheet 'BeamFactor' cell of (" << irow << "," << icol << ") is NULL";
            }
        }else{
            qDebug() << "sheet 'BeamFactor' not found";
        }


    }else{
        qDebug() << "[Cyntec::initBeamData]xlsReader error: ";
    }
}

void Cyntec::getBeamFactorDatas(int beamFactorID)
{
    if (!mBeamFactorData->isEmpty()){
        if (mBeamFactorData->contains(beamFactorID)){
            CyntecBeamFactorData data = mBeamFactorData->value(beamFactorID, CyntecBeamFactorData());
            emit updateBeamFactorData(data.elementMap, data.attDb,
                                      data.azimuth3dB_BW, data.elevation3dB_BW);
        }else{
            qDebug() << "mBeamFactorData do not have: " << beamFactorID;
        }
    }else{
        qDebug() << "mBeamFactorData is isEmpty";
    }
}

void Cyntec::getBeamTableData(int beamTableID)
{
    if (!mBeamTableData->isEmpty()){
        if (mBeamTableData->contains(beamTableID)){
            CyntecBeamTableData data = mBeamTableData->value(beamTableID);
            emit updateBeamTableData(data.azDeg, data.elDeg,
                                     data.azimuth3dB_BW, data.elevation3dB_BW);
        }else{
            qDebug() << "mBeamTableData do not have: " << beamTableID;
        }
    }else{
        qDebug() << "mBeamTableData is isEmpty";
    }
}

QVector<QVector<double>> Cyntec::getBeamTableDatas(int limitid)
{
    QVector<QVector<double>> data;
    for (auto key: mBeamTableData->keys()){
        CyntecBeamTableData d = mBeamTableData->value(key);
        if (limitid==d.beamtableId){
            break;
        }
        data.append({d.beamtableId, d.azDeg, d.elDeg});
    }
    return data;
}

QVector<QVector<double> > Cyntec::getBeamTableDatas(QString beamtype)
{
    QVector<QVector<double>> data;
    if (mBeamTypeData.contains(beamtype)){
        QList<int> ids = mBeamTypeData.value(beamtype);
        for (const int &id : ids) {
            if (mBeamTableData->contains(id)){
                CyntecBeamTableData d = mBeamTableData->value(id);
                data.append({d.beamtableId, d.azDeg, d.elDeg});
            }
        }
    }else{
        qDebug() << "No mBeamTypeData of " << beamtype;
    }
    return data;
}

BeamTypeRange Cyntec::getBeamTypeRange(QString beamtype)
{
    return mBeamTypeRangeData.value(beamtype, {0,0,0,0});
}

CyntecBeamFactorData Cyntec::getBeamFactorRange(int beamfactor)
{
    if (mBeamFactorData->contains(beamfactor)){
        CyntecBeamFactorData data = mBeamFactorData->value(beamfactor);
        return data;
    }
    return CyntecBeamFactorData();
}

int Cyntec::db2att(double db)
{
    // db: 0~32 (0.25 step)
    // att: 0~128
    if ((db<0)||(db>32)){
        qDebug() << "Att value out of range 0~32 value:" << QString::number(db);
        return -1;
    }
    //convert db to att value for setting in
    // 0, 0.25, 0.5, 0.75, 1 ...
    // Tx Att: 1db
    // spidev_test -D /dev/spidev2.0 -A "0:4:4"
    return qRound(db/0.25);
}

int Cyntec::findClosestBeamID(double targetAz, double targetEl,
                              QString beamtype, int beamfactor)
{

    int closestID = -1;
    double minDistance = std::numeric_limits<double>::max();
    // Narrow beam
    BeamTypeRange r= getBeamTypeRange(beamtype);
    // qDebug() << " beamtype: " << beamtype << " range:" << r.minAz << "," << r.maxAz;
    // << r.minEl << r.maxEl;
    CyntecBeamFactorData f = getBeamFactorRange(beamfactor);
    qDebug() << " HPBW az:" << f.azimuth3dB_BW << " el: " << f.elevation3dB_BW;

    // check if targetAz/El out of range
    double limitAz = r.minAz - f.azimuth3dB_BW/2;
    double limitMaxAz = r.maxAz + f.azimuth3dB_BW/2;
    if ((targetAz < limitAz)|| (targetAz > limitMaxAz)){
        qDebug() << "Az out of range:" << limitAz << " < " << targetAz << " < " << limitMaxAz;
        return closestID;
    }
    double limitEl = r.minEl - f.elevation3dB_BW/2;
    double limitMaxEl = r.maxEl + f.elevation3dB_BW/2;
    if ((targetEl < limitEl)|| (targetEl > limitMaxEl)){
        qDebug() << "El out of range:" << limitEl << " < " << targetEl << " < " << limitMaxEl;
        return closestID;
    }

    // find Closest BeamID
    CyntecBeamTableData btdata;
    foreach (int id, mBeamTypeData.value(beamtype)){
        btdata = mBeamTableData->value(id);
        double distance = MyFunc::euclideanDistance(btdata.azDeg, btdata.elDeg,
                                                    targetAz, targetEl);
        if (distance < minDistance) {
            minDistance = distance;
            closestID = id;
        }
    }
    return closestID;
}

QVector<int> Cyntec::findNearestNeighbors(int targetID, QString beamtype, int neighborGroup)
{
    QVector<BeamDistance> distances;

    if (!mBeamTableData->contains(targetID)){
        return {};
    }

    const CyntecBeamTableData& target = mBeamTableData->value(targetID);
    QString btype;
    for (auto it = mBeamTableData->constBegin(); it != mBeamTableData->constEnd(); ++it) {
        if (it.key() == targetID){
            continue;
        }
        btype = it.value().sBeamtype;
        if (!btype.startsWith(beamtype)) {
            qDebug() << QString::number(targetID)
                     << " Not correct beamtype, ignore:" << btype << " expect:" << beamtype;
            continue;
        }
        double d = std::sqrt(std::pow(it.value().azDeg - target.azDeg, 2) +
                             std::pow(it.value().elDeg - target.elDeg, 2));

        qDebug() << it.key() << " distances: " << QString::number(d) ;
        distances.append({it.key(), d});
    }

    std::sort(distances.begin(), distances.end(),
              [](const BeamDistance& a, const BeamDistance& b) {
                  return a.distance < b.distance;
              });

    // 收集前兩個不同 distance 的群組
    QVector<int> result;
    QSet<double> seenDistances;

    for (const BeamDistance& bd : distances) {
        if (seenDistances.size() >= neighborGroup && !seenDistances.contains(bd.distance))
            break;

        seenDistances.insert(bd.distance);
        result.append(bd.id);
    }

    qDebug() << "findNearestNeighbors:" << result;
    return result;
}

double Cyntec::getAz(int BeamID)
{
    double az = -1;
    if (mBeamTableData->contains(BeamID)){
        CyntecBeamTableData btdata = mBeamTableData->value(BeamID);
        az = btdata.azDeg;
    }
    return az;
}

double Cyntec::getTargetEIRP(double dist)
{
    //expect EIRP by dist (meter)
    double eirp=0.0;
    if (!mRangeDataObj.isEmpty()){
        QStringList skeys = sorted(mRangeDataObj.keys());
        foreach(const QString& key, skeys) {
            if (dist > key.toDouble()){
                auto d = mRangeDataObj.value(key).toObject();
                eirp = d.value("TargetEIRP").toDouble();
            }else {
                break;
            }
        }
    }else {
        qDebug() << "No mRangeDataObj";
    }
    return eirp;
}

int Cyntec::getBestBeamID(double minaz, double maxaz, double minel, double maxel)
{
    Q_UNUSED(minel)
    Q_UNUSED(maxel)
    double spAz = maxaz - minaz;
    if (spAz > 180){
        spAz = 360 - spAz;
    }
    qDebug() << "//TODO:Hanwha getBestBeamID" ;
    return 99;
}

QVector<double> Cyntec::getTxAtt(double dist)
{
    QVector<double> ds;
    double att1=0.0;
    double att2=0.0;
    if (!mRangeDataObj.isEmpty()){
        QStringList skeys = sorted(mRangeDataObj.keys());
        foreach(const QString& key, skeys) {
            if (dist > key.toDouble()){
                auto d = mRangeDataObj.value(key).toObject();
                att1 = d.value("TX1ATT").toDouble();
                att2 = d.value("TX2ATT").toDouble();
            }else {
                break;
            }
        }
    }else {
        qDebug() << "getTxAtt: No mRangeDataObj";
    }
    ds.append(att1);
    ds.append(att2);
    return ds;
}

QVector<double> Cyntec::getRxAtt(double dist)
{
    QVector<double> ds;
    double att1=0.0;
    double att2=0.0;
    if (!mRangeDataObj.isEmpty()){
        QStringList skeys = sorted(mRangeDataObj.keys());
        foreach(const QString& key, skeys) {
            if (dist > key.toDouble()){
                auto d = mRangeDataObj.value(key).toObject();
                att1 = d.value("RX1ATT").toDouble();
                att2 = d.value("RX2ATT").toDouble();
            }else {
                break;
            }
        }
    }else {
        qDebug() << "getRxAtt: No mRangeDataObj";
    }
    ds.append(att1);
    ds.append(att2);
    return ds;
}

QVector<double> Cyntec::getBFAtt(double dist)
{
    QVector<double> ds;
    double att1=0.0;
    double att2=0.0;
    if (!mRangeDataObj.isEmpty()){
        QStringList skeys = sorted(mRangeDataObj.keys());
        foreach(const QString& key, skeys) {
            if (dist > key.toDouble()){
                auto d = mRangeDataObj.value(key).toObject();
                att1 = d.value("BF1ATT").toDouble();
                att2 = d.value("BF2ATT").toDouble();
            }else {
                break;
            }
        }
    }else {
        qDebug() << "getBFAtt: No mRangeDataObj";
    }
    ds.append(att1);
    ds.append(att2);
    return ds;
}

QString Cyntec::getCmd(QString key)
{
    if (cmdObj.contains(key)){
        return cmdObj.value(key).toString();
    }else{
        qDebug() << "Cyntec::getCmd: NO command of " << key;
        return "";
    }
}

QList<int> Cyntec::getIntList(QString key)
{
    if (cmdObj.contains(key)){
        QList<int> intList;
        QJsonArray jsonArray = cmdObj.value(key).toArray();
        for (const QJsonValue &value : jsonArray) {
            if (value.isDouble()) {
                intList.append(value.toInt());
            }
        }
        return intList;
    }else{
        qDebug() << "Cyntec::getCmd: NO command of " << key;
        return QList<int>();
    }
}
