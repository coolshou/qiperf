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
    virtual ~AIP();
    virtual void initBeamData(QString filename);
    virtual QString getCmd(QString key) = 0;
    virtual QList<int> getIntList(QString key) = 0;
    virtual int findClosestBeamID(double targetAz, double targetEl,
                                  QString beamtype="", int beamfactor=1) = 0;
    virtual int getBestBeamID(double minaz, double maxaz,
                              double minel, double maxel) = 0;
    virtual QVector<double> getTxAtt(double distance) = 0;
    virtual QVector<double> getRxAtt(double distance) = 0;
    virtual QVector<double> getBFAtt(double distance) = 0;

    QStringList sorted(QStringList datas);
signals:
};

#endif // AIP_H
