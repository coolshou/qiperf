#include "cyntec.h"

#include <QFile>
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxworkbook.h"
using namespace QXlsx;

#include <QDebug>

Cyntec::Cyntec(QObject *parent)
    : AIP{parent}
{
    mBeamFactorData = QMap<int, CyntecBeamFactorData>();
}

void Cyntec::initBeamData(QString filename)
{
    QFile f(filename);
    if (!f.exists()){
        qDebug() << "File not exist: " << filename;
        return;
    }
    QXlsx::Document xlsReader(filename);
    if(xlsReader.load()){
        //BeamFactor
        if (xlsReader.selectSheet("BeamFactor")){
            QVariant varB, varC, varD, varE, varF;
            int irow=1;
            int icol=2;
            varB = xlsReader.read(irow, icol);
            while (varB.isValid()){
                irow++;
                varB =  xlsReader.read(irow, icol);
                if(varB.isValid()){
                    varC = xlsReader.read(irow, 3);
                    if (!varC.isValid()){
                        varC = "";
                    }
                    varD = xlsReader.read(irow, 4);
                    if (!varD.isValid()){
                        varD = 0;
                    }
                    varE = xlsReader.read(irow, 5);
                    if (!varE.isValid()){
                        varE = 0.0;
                    }
                    varF = xlsReader.read(irow, 6);
                    if (!varF.isValid()){
                        varF = 0.0;
                    }
                    mBeamFactorData.insert(varB.toInt(),
                                           CyntecBeamFactorData(varB.toInt(),
                                                                varC.toString(),
                                                                varD.toInt(),
                                                                varE.toDouble(),
                                                                varF.toDouble()));
                }

            }
            qDebug() << "mBeamFactorData.keys:" << mBeamFactorData.keys();
            QStringList factorkeys;
            for (int factorkey : mBeamFactorData.keys()) {
                // Convert the integer to a QString and add it to stringList
                factorkeys.append(QString::number(factorkey));
            }
            if (factorkeys.length()>0){
                emit newBeamFactorIDs(factorkeys);
            }
            //
        }else{
            qDebug() << "sheet 'BeamFactor' not found";
        }

        //BeamTable
        if (xlsReader.selectSheet("BeamTable")){
            QVariant varB, varC, varD, varE, varF;
            int irow=2;
            int icol=2;
            varB = xlsReader.read(irow, icol);
            while (varB.isValid()){
                irow++;
                varB =  xlsReader.read(irow, icol);
                if(varB.isValid()){
                    varC = xlsReader.read(irow, 3);
                    if (!varC.isValid()){
                        varC = 0;
                    }
                    varD = xlsReader.read(irow, 4);
                    if (!varD.isValid()){
                        varD = 0;
                    }
                    varE = xlsReader.read(irow, 5);
                    if (!varE.isValid()){
                        varE = 0.0;
                    }
                    varF = xlsReader.read(irow, 6);
                    if (!varF.isValid()){
                        varF = 0.0;
                    }
                    mBeamTableData.insert(varB.toInt(),
                                          CyntecBeamTableData(varB.toInt(),
                                                              varC.toInt(),
                                                              varD.toInt(),
                                                              varE.toDouble(),
                                                              varF.toDouble()));
                }

            }
            qDebug() << "mBeamFactorData.keys:" << mBeamTableData.keys();
            QStringList tablekeys;
            for (int tablekey : mBeamTableData.keys()) {
                // Convert the integer to a QString and add it to stringList
                tablekeys.append(QString::number(tablekey));
            }
            if (tablekeys.length()>0){
                emit newBeamTableIDs(tablekeys);
            }
        }else{
            qDebug() << "sheet 'BeamTable' not found";
        }
    }else{
        qDebug() << "Read " << filename << " fail";
    }
}

void Cyntec::getBeamFactorDatas(int beamFactorID)
{
    if (mBeamFactorData.isEmpty()){
        if (mBeamFactorData.contains(beamFactorID)){
            CyntecBeamFactorData data = mBeamFactorData.value(beamFactorID, CyntecBeamFactorData());
            qDebug() << "getBeamFactorDatas:" << beamFactorID << " elementMap:" <<data.elementMap;
            emit updateBeamFactorData(data.elementMap, data.attDb,
                                      data.azimuth3dB_BW, data.elevation3dB_BW);
        }else{
            qDebug() << "mBeamFactorData do not have: " << beamFactorID;
        }
    }else{
        qDebug() << "mBeamFactorData is isEmpty";
    }
}

void Cyntec::getBeamTableDatas(int beamTableID)
{
    if (mBeamTableData.isEmpty()){
        if (mBeamTableData.contains(beamTableID)){
            CyntecBeamTableData data = mBeamTableData.value(beamTableID);
            qDebug() << "getBeamFactorDatas:" << beamTableID
                     << " azDeg:" << data.azDeg << " elDeg:" << data.elDeg;
            emit updateBeamTableData(data.azDeg, data.elDeg,
                                     data.azimuth3dB_BW, data.elevation3dB_BW);
        }else{
            qDebug() << "mBeamTableData do not have: " << beamTableID;
        }
    }else{
        qDebug() << "mBeamTableData is isEmpty";
    }
}
