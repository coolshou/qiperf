#ifndef BEAMTYPERANGE_H
#define BEAMTYPERANGE_H

struct BeamTypeRange
{
    double minAz;
    double maxAz;
    double minEl;
    double maxEl;
};
struct BeamFactorRange
{
    double HPBWAz;//3db Az deg
    double HPBWEl;//3db El Deg
    double Att;
};

#endif // BEAMTYPERANGE_H
