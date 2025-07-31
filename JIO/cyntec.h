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
signals:
    void newBeamFactorIDs(QStringList keys);
    void updateBeamFactorData(QString elementMap, int attDb, double azBW, double elBW);
    void updateBeamTableData(double az, double el, double azBW, double elBW);
    void newBeamTableIDs(QStringList keys);
private:
    QMap<int, CyntecBeamFactorData> *mBeamFactorData;
    QMap<int, CyntecBeamTableData> *mBeamTableData;
};

#endif // CYNTEC_H
