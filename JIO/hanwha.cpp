#include "hanwha.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QDebug>
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxworkbook.h"
using namespace QXlsx;

Hanwha::Hanwha(QObject *parent)
    : AIP{parent}
{
    mBeamTableData = new QMap<int, HanwhaBeamTableData>();
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
    int row=13;
    // int endrow=253;
    int colBeamDirID=1;
    int colAZ=3;
    int colEI=4;
    int colBeamType=5;
    QVariant varA, varC, varD, varE;
    QXlsx::Document xlsReader(filedevice);
    if(xlsReader.load()){
        mBeamTableData->clear();
        mBeamtypes.clear();
        // for(int i=row; i<=endrow;i++){
        auto cell = xlsReader.cellAt(row, colBeamDirID); // col A
        if ( cell != NULL )
        {
            varA = cell->readValue();
            while (varA.isValid()){
                QCoreApplication::processEvents(QEventLoop::AllEvents);
                row++;
                cell = xlsReader.cellAt(row, colBeamDirID); // col A
                if ( cell != NULL )
                {
                    varA = cell->readValue(); // read cell value (number(double), QDateTime, QString ...)
                    cell = xlsReader.cellAt(row, colAZ); // col C : AZ
                    if ( cell != NULL )
                    {
                        varC = cell->readValue(); // read cell value (number(double), QDateTime, QString ...)
                    }else{
                        qDebug() << "get cell " << row << " x " << colAZ << " fail";
                    }
                    cell = xlsReader.cellAt(row, colEI); // col D : EI
                    if ( cell != NULL )
                    {
                        varD = cell->readValue(); // read cell value (number(double), QDateTime, QString ...)
                    }else{
                        qDebug() << "get cell " << row << " x " << colEI << " fail";
                    }
                    cell = xlsReader.cellAt(row, colBeamType); // col E : beamtype
                    if ( cell != NULL )
                    {
                        varE = cell->readValue(); // read cell value (number(double), QDateTime, QString ...)
                        if (!mBeamtypes.contains(varE.toString())){
                            mBeamtypes.append(varE.toString());
                        }
                    }else{
                        qDebug() << "get cell " << row << " x " << colBeamType << " fail";
                    }
                    mBeamTableData->insert(varA.toInt(), HanwhaBeamTableData(varA.toInt(),
                                                                             varC.toDouble(),
                                                                             varD.toDouble(),
                                                                             varE.toString()));
                }else{
                    break;
                }
            }
            QStringList tablekeys;
            for (int tablekey : mBeamTableData->keys()) {
                // Convert the integer to a QString and add it to stringList
                tablekeys.append(QString::number(tablekey));
            }
            if (tablekeys.length()>0){
                emit newBeamTableIDs(tablekeys);
            }
            if (mBeamtypes.length()>0){
                emit updateBeamTypes(mBeamtypes);
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

QVector<QVector<double>> Hanwha::getBeamTableDatas(int limitid)
{
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
