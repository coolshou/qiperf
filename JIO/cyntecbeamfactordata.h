#ifndef CYNTECBEAMFACTORDATA_H
#define CYNTECBEAMFACTORDATA_H

#include <QString> // For "Element Map" (e.g., "8x8", "8x4")

// This struct will hold all the data for a single BeamFactorID
struct CyntecBeamFactorData
{
    // The BeamFactorID itself is often the key in a map,
    // so it might not be strictly necessary here, but including it
    // can make the struct self-contained if you iterate through a list.
    int beamFactorId;

    QString elementMap;
    double attDb; // Assuming Att (dB) is an integer
    double azimuth3dB_BW; // Assuming Azimuth 3dB BW (°) is a double
    double elevation3dB_BW; // Assuming Elevation 3dB BW (°) is a double

    // Optional: A constructor for easy initialization
    CyntecBeamFactorData(int id = 0, const QString& map = "", int att = 0,
                   double az = 0.0, double el = 0.0)
        : beamFactorId(id), elementMap(map), attDb(att),
        azimuth3dB_BW(az), elevation3dB_BW(el)
    {}
};

#endif // CYNTECBEAMFACTORDATA_H
