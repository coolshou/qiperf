#ifndef CYNTEC_H
#define CYNTEC_H

#include <QObject>
#include <QHash>
#include <QList>

#include "aip.h"
#include "cyntecbeamfactordata.h"
#include "cyntecbeamtabledata.h"

class Cyntec : public AIP
{
    Q_OBJECT
public:
    explicit Cyntec(QObject *parent = nullptr);
    void initBeamData(QString filename) override;
    void initBeamData(QIODevice *device);
    void getBeamFactorDatas(int beamFactorID);
    void getBeamTableData(int beamTableID);
    QVector<QVector<double>> getBeamTableDatas(int limitid=95); //[[id,az,el],[]]...
    QVector<QVector<double>> getBeamTableDatas(QString beamtype);
    int db2att(double db);
signals:
    void updateRefFile(QString filename);
    void newBeamFactorIDs(QStringList keys);
    void updateBeamFactorData(QString elementMap, int attDb, double azBW, double elBW);
    void updateBeamTableData(double az, double el, double azBW, double elBW);
    void newBeamTableIDs(QStringList keys);
    void updateBeamTypes(QStringList beamtypes);
    void updateBeamTypeGroup(QMap<QString, QStringList> data);
private:
    QMap<int, CyntecBeamFactorData> *mBeamFactorData;
    QMap<int, CyntecBeamTableData> *mBeamTableData;
    // QStringList mBeamtypes;
    QMap<QString, QStringList> mBeamTypeData;
};

#endif // CYNTEC_H
