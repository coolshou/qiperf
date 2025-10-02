#ifndef HANWHABEAMTABLEDATA_H
#define HANWHABEAMTABLEDATA_H

#include <QString>

struct HanwhaBeamTableData
{
    double beamtableId;

    double azDeg; // AZ degree
    double elDeg; // EL degree
    QString sBeamtype;

    HanwhaBeamTableData(double id = 0.0, double az = 0.0, double el = 0.0, QString beamtype="")
        :beamtableId(id), azDeg(az), elDeg(el), sBeamtype(beamtype)
    {}
};
#endif // HANWHABEAMTABLEDATA_H
