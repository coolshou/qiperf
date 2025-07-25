#ifndef CYNTECBEAMTABLEDATA_H
#define CYNTECBEAMTABLEDATA_H


// This struct will hold all the data for a single BeamFactorID
struct CyntecBeamTableData
{
    // The BeamFactorID itself is often the key in a map,
    // so it might not be strictly necessary here, but including it
    // can make the struct self-contained if you iterate through a list.
    int beamtableId;

    int azDeg; // AZ degree
    int elDeg; // EL degree
    double azimuth3dB_BW; // Assuming Azimuth 3dB BW (°) is a double
    double elevation3dB_BW; // Assuming Elevation 3dB BW (°) is a double

    // Optional: A constructor for easy initialization
    CyntecBeamTableData(int id = 0, int az = 0, int el = 0,
                        double azimuth = 0.0, double elevation = 0.0)
        : beamtableId(id), azDeg(az), elDeg(el),
        azimuth3dB_BW(azimuth), elevation3dB_BW(elevation)
    {}
};
#endif // CYNTECBEAMTABLEDATA_H
