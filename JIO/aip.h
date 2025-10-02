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
class AIP : public QObject
{
    Q_OBJECT
public:
    enum ModuleType{
        Unknown,
        Cyntec,
        Hanwha,
    };
    Q_ENUM(ModuleType)

    explicit AIP(QObject *parent = nullptr);
    virtual void initBeamData(QString filename);
    QStringList sorted(QStringList datas);
signals:
};

#endif // AIP_H
