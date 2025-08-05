#ifndef HANWHABEAMTABLEDATA_H
#define HANWHABEAMTABLEDATA_H

#include <QString>

struct HanwhaBeamTableData
{
    int beamtableId;

    double azDeg; // AZ degree
    double elDeg; // EL degree
    QString beamtypeName;

    HanwhaBeamTableData(int id = 0, double az = 0.0, double el = 0.0, QString beamtype="")
        :beamtableId(id), azDeg(az), elDeg(el), beamtypeName(beamtype)
    {}
};
#endif // HANWHABEAMTABLEDATA_H
