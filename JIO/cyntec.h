#ifndef CYNTEC_H
#define CYNTEC_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QJsonObject>

#include "aip.h"
#include "cyntecbeamfactordata.h"
#include "cyntecbeamtabledata.h"
#include "beamtyperange.h"
#include "beamdistance.h"

class Cyntec : public AIP
{
    Q_OBJECT
public:
    explicit Cyntec(QObject *parent = nullptr);
    void initCmds();
    void initBeamData(QString filename) override;
    void initBeamData(QIODevice *device);
    void getBeamFactorDatas(int beamFactorID);
    void getBeamTableData(int beamTableID);
    QVector<QVector<double>> getBeamTableDatas(int limitid=95); //[[id,az,el],[]]...
    QVector<QVector<double>> getBeamTableDatas(QString beamtype);
    BeamTypeRange getBeamTypeRange(QString beamtype);
    int db2att(double db);
    int findClosestBeamID(double targetAz, double targetEl, QString beamtype="Narrow");
    QVector<int> findNearestNeighbors(int targetID, QString beamtype="Narrow",
                                      int neighborCount = 6);
    double getAz(int BeamID);
signals:
    void updateRefFile(QString filename);
    void newBeamFactorIDs(QStringList keys);
    void updateBeamFactorData(QString elementMap, int attDb, double azBW, double elBW);
    void updateBeamTableData(double az, double el, double azBW, double elBW);
    void newBeamTableIDs(QStringList keys);
    void updateBeamTypes(QStringList beamtypes);
    void updateBeamTypeGroup(QMap<QString, QList<int>> data);
private:
    QMap<int, CyntecBeamFactorData> *mBeamFactorData;
    QMap<int, CyntecBeamTableData> *mBeamTableData;
    // QStringList mBeamtypes;
    QMap<QString, QList<int>> mBeamTypeData; // beam type => list of ID
    QMap<QString, BeamTypeRange> mBeamTypeRangeData;
    QJsonObject cmdObj;
};

#endif // CYNTEC_H
