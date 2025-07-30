#ifndef HANWHABEAMTABLEDATA_H
#define HANWHABEAMTABLEDATA_H

struct HanwhaBeamTableData
{
    double beamtableId;

    double azDeg; // AZ degree
    double elDeg; // EL degree

    HanwhaBeamTableData(int id = 0.0, double az = 0.0, double el = 0.0)
        :beamtableId(id), azDeg(az), elDeg(el)
    {}
};
#endif // HANWHABEAMTABLEDATA_H
