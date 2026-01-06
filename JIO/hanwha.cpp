#include "hanwha.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QDebug>
#include <QJsonParseError>
#include <QJsonDocument>

#include "myfunc.h"

#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxworkbook.h"
using namespace QXlsx;

Hanwha::Hanwha(QObject *parent)
    : AIP{parent}
{
    mBeamTableData = new QMap<int, HanwhaBeamTableData>();
    // mBeamTypeRangeData = new QMap<QString, BeamTypeRange>();
    // mBeamTypeData = new QMap<QString, QList<int>>();
    mRangeDataObj = QJsonObject();
    initCmds();
}

void Hanwha::initCmds()
{
    QFile fobj(":/aas/hanwha");
    if (fobj.open(QIODevice::ReadOnly)) {
        QByteArray jsonData = fobj.readAll();
        fobj.close();

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
        // data from 20250829.xlsx
        mRangeDataObj = cmdObj.value("DistanceData").toObject();

        // qDebug() << "mRangeDataObj:" << mRangeDataObj;
    }else {
        qDebug() << "Failed to open " << fobj.fileName() << " for reading:" << fobj.errorString();
    }
}

void Hanwha::initBeamData(QString filename)
{
    /*
    beamData["0-0"] = {0,0, "spoiled_60"};
    beamData["0-1"] = {0,0, "spoiled_45"};
    beamData["0-2"] = {0,0, "spoiled_30"};
    beamData["1"] = {0,0, "normal"};
    ...
    beamData["239"] = {21.272, 20.795, "normal"};
*/
    QFile readFile(filename);
    if (!readFile.exists()){
        qDebug() << "File not exist: " << filename;
        return;
    }
    if (readFile.open(QIODevice::ReadOnly)) {
        qDebug() << "\nCalling initBeamData with QFile...";
        initBeamData(&readFile); // Pass the address of the QFile object
        readFile.close(); // Close the file after initBeamData is done
        qDebug() << "File closed after initBeamData.";
    } else {
        qDebug() << "Failed to open" << filename << "for reading:" << readFile.errorString();
    }
}

void Hanwha::initBeamData(QIODevice *filedevice)
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
    int row=13;
    // int endrow=253;
    int colBeamDirID=1;
    int colAZ=3;
    int colEI=4;
    int colBeamType=5;
    QVariant varBeamDirID, varAz, varEl, varBeamType;
    QXlsx::Document xlsReader(filedevice);
    if(xlsReader.load()){
        mBeamTableData->clear();
        mBeamTypeData.clear();
        // for(int i=row; i<=endrow;i++){
        auto cell = xlsReader.cellAt(row, colBeamDirID); // col A
        if ( cell != NULL )
        {
            varBeamDirID = cell->readValue();
            while (varBeamDirID.isValid()){
                QCoreApplication::processEvents(QEventLoop::AllEvents);
                row++;
                cell = xlsReader.cellAt(row, colBeamDirID); // col A
                if ( cell != NULL )
                {
                    varBeamDirID = cell->readValue();
                    cell = xlsReader.cellAt(row, colAZ); // col C : AZ
                    if ( cell != NULL )
                    {
                        varAz = cell->readValue();
                    }else{
                        qDebug() << "get cell " << row << " x " << colAZ << " fail";
                    }
                    cell = xlsReader.cellAt(row, colEI); // col D : EI
                    if ( cell != NULL )
                    {
                        varEl = cell->readValue();
                    }else{
                        qDebug() << "get cell " << row << " x " << colEI << " fail";
                    }
                    cell = xlsReader.cellAt(row, colBeamType); // col E : beamtype
                    if ( cell != NULL )
                    {
                        varBeamType = cell->readValue();
                        if (!mBeamTypeData.contains(varBeamType.toString())){
                            mBeamTypeData[varBeamType.toString()]=QList<int>();
                        }
                        mBeamTypeData[varBeamType.toString()].append(varBeamDirID.toInt());
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
                    }else{
                        qDebug() << "get cell " << row << " x " << colBeamType << " fail";
                    }
                    // qDebug() << "varBeamDirID:" << varBeamDirID.toString() << " varBeamType:" << varBeamType.toString();
                    mBeamTableData->insert(varBeamDirID.toInt(), HanwhaBeamTableData(varBeamDirID.toInt(),
                                                                             varAz.toDouble(),
                                                                             varEl.toDouble(),
                                                                             varBeamType.toString()));
                }else{
                    break;
                }
            }
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
        }else{
            qDebug() << "data format is wrong";
        }
    }else{
        qDebug() << "[Hanwha::initBeamData]xlsReader error: ";
    }
}

void Hanwha::getBeamTableData(int beamTableID)
{
    if (!mBeamTableData->isEmpty()){
        if (mBeamTableData->contains(beamTableID)){
            HanwhaBeamTableData data = mBeamTableData->value(beamTableID);
            // qDebug() << "Hanwha::getBeamTableData:" << beamTableID
            //          << " azDeg:" << data.azDeg
            //          << " elDeg" << data.elDeg;
            //TODO: double azBW, double elBW value
            emit updateBeamTableData(data.azDeg, data.elDeg, 0, 0);
        }else{
            qDebug() << "BeamTableData fo not have key:" << beamTableID;
        }
    }else{
        qDebug() << "BeamTableData is empty";
    }
}

BeamTypeRange Hanwha::getBeamTypeRange(QString beamtype)
{
    return mBeamTypeRangeData.value(beamtype, {0,0,0,0});
}

int Hanwha::findClosestBeamID(double targetAz, double targetEl, QString beamtype, int beamfactor)
{
    Q_UNUSED(beamfactor)
    int closestID = -1;
    double minDistance = std::numeric_limits<double>::max();
    // Narrow beam
    BeamTypeRange r= getBeamTypeRange(beamtype);
    qDebug() << "//TODO: [findClosestBeamID] azimuth3dB_BW , elevation3dB_BW:"
             << "beamtype:" << beamtype
             << "maxAz:" << r.maxAz << " minAz:" << r.minAz
             << "maxEl:" << r.maxEl << " minEl:" << r.minEl;
    double azBW=0;
    double elBW=0;

    // check if targetAz/El out of range
    double limitAz = r.minAz - azBW/2;
    double limitMaxAz = r.maxAz + azBW/2;
    if ((targetAz < limitAz)|| (targetAz > limitMaxAz)){
        qDebug() << "Az out of range:" << limitAz << " < " << targetAz << " < " << limitMaxAz;
        return closestID;
    }
    double limitEl = r.minEl - elBW/2;
    double limitMaxEl = r.maxEl + elBW/2;
    if ((targetEl < limitEl)|| (targetEl > limitMaxEl)){
        qDebug() << "El out of range:" << limitEl << " < " << targetEl << " < " << limitMaxEl;
        return closestID;
    }
    // find Closest BeamID
    HanwhaBeamTableData btdata;
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

QVector<QVector<double>> Hanwha::getBeamTableDatas(int limitid)
{
    //all BeamTableDatas
    QVector<QVector<double>> data;
    for (auto key : mBeamTableData->keys()){
        HanwhaBeamTableData d = mBeamTableData->value(key);
        if (d.beamtableId == limitid) {
            break;
        }
        data.append({d.beamtableId, d.azDeg, d.elDeg});
    }
    return data;
}

QVector<QVector<double>> Hanwha::getBeamTableDatas(QString beamtype)
{
    QVector<QVector<double>> data;
    if (mBeamTypeData.contains(beamtype)){
        QList<int> ids = mBeamTypeData.value(beamtype);
        for (const int &id : ids) {
            if (mBeamTableData->contains(id)){
                HanwhaBeamTableData d = mBeamTableData->value(id);
                data.append({d.beamtableId, d.azDeg, d.elDeg});
            }
        }
    }else{
        qDebug() << "No mBeamTypeData of " << beamtype;
    }
    return data;
}

QVector<int> Hanwha::findNearestNeighbors(int targetID, QString beamtype, int neighborGroup)
{
    QVector<BeamDistance> distances;

    if (!mBeamTableData->contains(targetID)){
        return {};
    }

    const HanwhaBeamTableData& target = mBeamTableData->value(targetID);
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

int Hanwha::getBestBeamID(double minaz, double maxaz, double minel, double maxel)
{
    Q_UNUSED(minel)
    Q_UNUSED(maxel)
    double spAz = maxaz - minaz;
    if (spAz > 180){
        spAz = 360 - spAz;
    }
    qDebug() << "//TODO:Hanwha getBestBeamID" ;

    //"NERROW" HPBWaz, HPBWel?
    // BeamTypeRange r = mBeamTypeRangeData.value("NERROW");
    // if ((r.maxAz < maxaz)||(r.minAz > minaz)){
    //     //out of range;
    //     qDebug() << "getBestBeamID: ("<< minaz << "," << maxaz <<")"
    //              << "Az out of range:" <<r.minAz << "," << r.maxAz;
    //     //"widwbeam"
    // }else{

    // }

    // TODO: other type?
    return 477;
}

QList<int> Hanwha::getIntList(QString key)
{
    Q_UNUSED(key)
    // TODO: getIntList?
}

double Hanwha::getTargetEIRP(double dist)
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

QVector<double> Hanwha::getRxAtt(double distance)
{
    QVector<double> ds;
    double att1 = 0.0;
    double att2 = 0.0;
    double lan = 0.0;
    if (!mRangeDataObj.isEmpty()){
        QStringList skeys = sorted(mRangeDataObj.keys());
        foreach(const QString& key, skeys) {
            if (distance > key.toDouble()){
                auto d = mRangeDataObj.value(key).toObject();
                att1 = d.value("IFRX1ATT").toDouble();
                att2 = d.value("IFRX2ATT").toDouble();
                lan = d.value("LNARXATT").toDouble();
            }else {
                break;
            }
        }
    }else {
        qDebug() << "getRxAtt: No mRangeDataObj";
    }
    ds.append(att1);
    ds.append(att2);
    ds.append(lan);
    return ds;
}

QVector<double> Hanwha::getBFAtt(double distance)
{
    QVector<double> ds;
    ds = getBFTxAtt(distance);
    ds.append(getBFRxAtt(distance));
    return ds;
}

QVector<double> Hanwha::getBFRxAtt(double distance)
{
    QVector<double> ds;
    double att1=0.0;
    double att2=0.0;
    if (!mRangeDataObj.isEmpty()){
        QStringList skeys = sorted(mRangeDataObj.keys());
        foreach(const QString& key, skeys) {
            if (distance > key.toDouble()){
                auto d = mRangeDataObj.value(key).toObject();
                att1 = d.value("BFRX1ATT").toDouble();
                att2 = d.value("BFRX2ATT").toDouble();
            }else {
                break;
            }
        }
    }else {
        qDebug() << "getBFRxAtt: No mRangeDataObj";
    }
    ds.append(att1);
    ds.append(att2);
    return ds;
}

QVector<double> Hanwha::getTxAtt(double distance)
{
    QVector<double> ds;
    double att1 = 0.0;
    double att2 = 0.0;
    if (!mRangeDataObj.isEmpty()){
        QStringList skeys = sorted(mRangeDataObj.keys());
        foreach(const QString& key, skeys) {
            if (distance > key.toDouble()){
                auto d = mRangeDataObj.value(key).toObject();
                att1 = d.value("IFTX1ATT").toDouble();
                att2 = d.value("IFTX2ATT").toDouble();
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

QVector<double> Hanwha::getBFTxAtt(double distance)
{
    QVector<double> ds;
    double att1=0.0;
    double att2=0.0;
    if (!mRangeDataObj.isEmpty()){
        QStringList skeys = sorted(mRangeDataObj.keys());
        foreach(const QString& key, skeys) {
            if (distance > key.toDouble()){
                auto d = mRangeDataObj.value(key).toObject();
                att1 = d.value("BFTX1ATT").toDouble();
                att2 = d.value("BFTX2ATT").toDouble();
            }else {
                break;
            }
        }
    }else {
        qDebug() << "getBFTxAtt: No mRangeDataObj";
    }
    ds.append(att1);
    ds.append(att2);
    return ds;
}

QString Hanwha::getCmd(QString key)
{
    if (cmdObj.contains(key)){
        return cmdObj.value(key).toString();
    }else{
        qDebug() << "Hanwha::getCmd: NO command of " << key;
        return "";
    }
}

QString Hanwha::getFreqIdx(double ghz)
{
    // turn freq to idx which Hanwha support
    QString idx="";
    QJsonObject fs = cmdObj.value("Freq").toObject();
    foreach (auto key, fs.keys()){
        if (fs.value(key).toDouble() ==ghz){
            idx = key;
            break;
        }
    }
    return idx;
}
