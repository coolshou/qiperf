#include "hanwha.h"

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
    int row=15;
    int endrow=253;

    QVariant varA, varC, varD, varE;
    // qDebug() << "xlsReader(filename): " << filename;
    QXlsx::Document xlsReader(filedevice);
    if(xlsReader.load()){
        mBeamTableData->clear();
        for(int i=row; i<=endrow;i++){
            Cell* cellA = xlsReader.cellAt(i, 1).get(); // col A
            if ( cellA != NULL )
            {
                varA = cellA->readValue(); // read cell value (number(double), QDateTime, QString ...)
                // qDebug() << varA; // display value. it is 'Hello Qt!'.
            }else{
                qDebug() << "get cell " << i << " x 1 fail";
                continue;
            }
            Cell* cellC = xlsReader.cellAt(i, 3).get(); // col C : AZ
            if ( cellC != NULL )
            {
                varC = cellC->readValue(); // read cell value (number(double), QDateTime, QString ...)
                // qDebug() << varC; // display value. it is 'Hello Qt!'.
            }else{
                qDebug() << "get cell " << i << " x 3 fail";
                continue;
            }
            Cell* cellD = xlsReader.cellAt(i, 4).get(); // col D : EI
            if ( cellD != NULL )
            {
                varD = cellD->readValue(); // read cell value (number(double), QDateTime, QString ...)
                // qDebug() << varD; // display value. it is 'Hello Qt!'.
            }else{
                qDebug() << "get cell " << i << " x 4 fail";
                continue;
            }
            Cell* cellE = xlsReader.cellAt(i, 5).get(); // col E : beamtype
            if ( cellE != NULL )
            {
                varE = cellE->readValue(); // read cell value (number(double), QDateTime, QString ...)
                // qDebug() << varE; // display value. it is 'Hello Qt!'.
            }else{
                qDebug() << "get cell " << i << " x 5 fail";
                continue;
            }
            mBeamTableData->insert(varA.toInt(), HanwhaBeamTableData(varA.toDouble(),
                                                                     varC.toDouble(),
                                                                     varD.toDouble()));
        }
        QStringList tablekeys;
        for (int tablekey : mBeamTableData->keys()) {
            // Convert the integer to a QString and add it to stringList
            tablekeys.append(QString::number(tablekey));
        }
        if (tablekeys.length()>0){
            emit newBeamTableIDs(tablekeys);
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
            qDebug() << "Hanwha::getBeamTableData:" << beamTableID
                     << " azDeg:" << data.azDeg
                     << " elDeg" << data.elDeg;
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
