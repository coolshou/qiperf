#ifndef HANWHA_H
#define HANWHA_H

#include <QObject>
#include "aip.h"
#include "hanwhabeamtabledata.h"
#include "beamtyperange.h"
#include "beamdistance.h"

class Hanwha : public AIP
{
    Q_OBJECT
public:
    explicit Hanwha(QObject *parent = nullptr);
    void getBeamTableData(int beamTableID);
    BeamTypeRange getBeamTypeRange(QString beamtype);
    int findClosestBeamID(double targetAz, double targetEl,
                          QString beamtype="NARROW", int beamfactor=1);
    QVector<QVector<double>> getBeamTableDatas(int limitid=240);
    QVector<QVector<double>> getBeamTableDatas(QString beamtype);
    QVector<int> findNearestNeighbors(int targetID, QString beamtype="NARROW",
                                      int neighborGroup = 1);
public slots:
    void initBeamData(QString filename);
    void initBeamData(QIODevice *device);

signals:
    void updateRefFile(QString filename);
    void newBeamTableIDs(QStringList keys);
    void updateBeamTableData(double az, double el, double azBW, double elBW);
    void updateBeamTypes(QStringList beamtypes);
    void updateBeamTypeGroup(QMap<QString, QList<int>> data);
private:
    QMap<int, HanwhaBeamTableData> *mBeamTableData;
    QMap<QString, QList<int>> mBeamTypeData;
    QMap<QString, BeamTypeRange> mBeamTypeRangeData;
};

#endif // HANWHA_H
