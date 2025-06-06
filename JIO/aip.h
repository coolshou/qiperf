#ifndef AIP_H
#define AIP_H

#include <QObject>
#include <QMap>

struct BeamLattice {
    double AZ;
    double EI;
    QString BeamType;
};

//Antenna-in-Package
class AiP : public QObject
{
    Q_OBJECT
public:
    explicit AiP(QObject *parent = nullptr);

signals:
};

#endif // AIP_H
