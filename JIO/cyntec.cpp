#include "cyntec.h"

#include <QFile>
#include <QCoreApplication>
#include <QEventLoop>
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxworkbook.h"
using namespace QXlsx;

#include <QDebug>

Cyntec::Cyntec(QObject *parent)
    : AIP{parent}
{
    mBeamFactorData = new QMap<int, CyntecBeamFactorData>();
    mBeamTableData = new QMap<int, CyntecBeamTableData>();
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
        QVariant varBeamID, varC, varD, varE, varF, varG;
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
                        varBeamID = cell->readValue();
                        if(varBeamID.isValid()){
                            cell = xlsReader.cellAt(irow, 3);
                            if (cell != NULL){
                                varC = cell->readValue(); //AZ
                            }
                            cell = xlsReader.cellAt(irow, 4);
                            if (cell != NULL){
                                varD = cell->readValue(); //EL
                            }
                            cell = xlsReader.cellAt(irow, 5);
                            if (cell != NULL){
                                varE = cell->readValue(); //Azimuth 3dB BW (°)
                            }
                            cell = xlsReader.cellAt(irow, 6);
                            if (cell != NULL){
                                varF = cell->readValue(); //Elevation 3dB BW (°)
                            }
                            cell = xlsReader.cellAt(irow, 7);
                            if (cell != NULL){
                                varG = cell->readValue(); //comment
                                if (!mBeamTypeData.contains(varG.toString())){
                                    mBeamTypeData[varG.toString()]=QStringList();
                                }
                                mBeamTypeData[varG.toString()].append(varBeamID.toString());
                            }
                            // qDebug() << "[TableData]varB:" << varB
                            //          << "varC:" << varC << " varD:" << varD
                            //          << "varE:" << varE << " varF:" << varF;
                            mBeamTableData->insert(varBeamID.toInt(),
                                                   CyntecBeamTableData(varBeamID.toInt(),
                                                                       varC.toDouble(),
                                                                       varD.toDouble(),
                                                                       varE.toDouble(),
                                                                       varF.toDouble()));
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
                            varC = cell->readValue(); // Element Map
                        }
                        cell = xlsReader.cellAt(irow, 4);
                        if (cell != NULL){
                            varD = cell->readValue(); // Att (dB)
                        }
                        cell = xlsReader.cellAt(irow, 5);
                        if (cell != NULL){
                            varE = cell->readValue(); // Azimuth 3dB BW (°)
                        }
                        cell = xlsReader.cellAt(irow, 6);
                        if (cell != NULL){
                            varF = cell->readValue(); // Elevation 3dB BW (°)
                        }
                        // qDebug() << "[FactorData]varB:" << varB
                        //          << "varC:" << varC << " varD:" << varD
                        //          << "varE:" << varE << " varF:" << varF;
                        mBeamFactorData->insert(varBeamID.toInt(),
                                                CyntecBeamFactorData(varBeamID.toInt(),
                                                                     varC.toString(),
                                                                     varD.toInt(),
                                                                     varE.toDouble(),
                                                                     varF.toDouble()));
                    }else{
                        // qDebug() << "[FactorData]cell is NULL " << irow << "," << icol;
                        break;
                    }
                }
                qDebug() << "mBeamFactorData.keys:" << mBeamFactorData->keys().length();
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
            // qDebug() << "getBeamTableData:" << beamTableID
            //          << " azDeg:" << data.azDeg << " elDeg:" << data.elDeg
            //          << " azBW:" << data.azimuth3dB_BW << " elBW:" << data.elevation3dB_BW;
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
        QStringList ids = mBeamTypeData.value(beamtype);
        for (const QString &id : ids) {
            if (mBeamTableData->contains(id.toInt())){
                CyntecBeamTableData d = mBeamTableData->value(id.toInt());
                data.append({d.beamtableId, d.azDeg, d.elDeg});
            }
        }
    }else{
        qDebug() << "No mBeamTypeData of " << beamtype;
    }
    return data;
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
