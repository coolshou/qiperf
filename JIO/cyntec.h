#ifndef CYNTEC_H
#define CYNTEC_H

#include <QObject>
#include <QHash>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>

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
    CyntecBeamFactorData getBeamFactorRange(int beamfactor);
    int db2att(double db);
    int findClosestBeamID(double targetAz, double targetEl,
                          QString beamtype="Narrow", int beamfactor=1);
    QVector<int> findNearestNeighbors(int targetID, QString beamtype="Narrow",
                                      int neighborGroup = 1);
    double getAz(int BeamID);
    double getTargetEIRP(double dist);
    int getBestBeamID(double minaz, double maxaz, double minel, double maxel);
    QVector<double> getTxAtt(double distance);
    QVector<double> getRxAtt(double distance);
    QVector<double> getBFAtt(double distance);
    QString getCmd(QString key);
    QList<int> getIntList(QString key);

signals:
    void updateRefFile(QString filename);
    void newBeamFactorIDs(QStringList keys);
    void updateBeamFactorData(QString elementMap, double attDb, double azBW, double elBW);
    void updateBeamTableData(double az, double el, double azBW, double elBW);
    void newBeamTableIDs(QStringList keys);
    void updateBeamTypes(QStringList beamtypes);
    void updateBeamTypeGroup(QMap<QString, QList<int>> data);
    void updateBeamFactorSupport(QMap<QString, QList<int>> data);
private:
    QMap<int, CyntecBeamFactorData> *mBeamFactorData; // id, CyntecBeamFactorData (FactorId, map, att, HPAz, HPEl)
    QMap<int, CyntecBeamTableData> *mBeamTableData; // id, CyntecBeamTableData (BeamID, Az, El, HPAz, HPEl, BeamType)
    // QStringList mBeamtypes;
    QMap<QString, QList<int>> mBeamTypeData; // beam type (Narrow, Spoiled, Tri) => list of ID
    QMap<QString, BeamTypeRange> mBeamTypeRangeData;
    QJsonObject cmdObj;
    QJsonObject mRangeDataObj;
    QMap<QString, QList<int>> mBeamFactorSupport; // diff BeamFactorID support diff Beam Direction ID (BeamTableID)
};

#endif // CYNTEC_H
