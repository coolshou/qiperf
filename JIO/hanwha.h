#ifndef HANWHA_H
#define HANWHA_H

#include <QObject>
#include "aip.h"

class Hanwha : public AiP
{
    Q_OBJECT
public:
    explicit Hanwha(QObject *parent = nullptr);
    void initBeamData(QString filename);
    void getBeamData();

private:
    QMap<QString, BeamLattice> beamData;
};

#endif // HANWHA_H
