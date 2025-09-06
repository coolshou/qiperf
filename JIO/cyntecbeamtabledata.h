#ifndef CYNTECBEAMTABLEDATA_H
#define CYNTECBEAMTABLEDATA_H

#include <QString>

// This struct will hold all the data for a single BeamFactorID
struct CyntecBeamTableData
{
    // The BeamFactorID itself is often the key in a map,
    // so it might not be strictly necessary here, but including it
    // can make the struct self-contained if you iterate through a list.
    double beamtableId;

    double azDeg; // AZ degree
    double elDeg; // EL degree
    double azimuth3dB_BW; // Assuming Azimuth 3dB BW (°) is a double
    double elevation3dB_BW; // Assuming Elevation 3dB BW (°) is a double
    QString sBeamtype;

    // Optional: A constructor for easy initialization
    CyntecBeamTableData(int id = 0.0, double az = 0.0, double el = 0.0,
                        double azimuth = 0.0, double elevation = 0.0,
                        QString beamtype="Narrow")
        : beamtableId(id), azDeg(az), elDeg(el),
        azimuth3dB_BW(azimuth), elevation3dB_BW(elevation), sBeamtype(beamtype)
    {}
};
#endif // CYNTECBEAMTABLEDATA_H
