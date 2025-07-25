#ifndef HANWHA_H
#define HANWHA_H

#include <QObject>
#include "aip.h"

class Hanwha : public AIP
{
    Q_OBJECT
public:
    explicit Hanwha(QObject *parent = nullptr);
    void initBeamData(QString filename);
    void initBeamData(QIODevice *device);
    void getBeamData();
signals:
    void newBeamTableIDs(QStringList keys);
private:
    QMap<QString, BeamLattice> beamData;
};

#endif // HANWHA_H
