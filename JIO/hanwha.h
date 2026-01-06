#ifndef HANWHA_H
#define HANWHA_H

#include <QObject>
#include <QJsonObject>

#include "aip.h"
#include "hanwhabeamtabledata.h"
#include "beamtyperange.h"
#include "beamdistance.h"

class Hanwha : public AIP
{
    Q_OBJECT
public:
    explicit Hanwha(QObject *parent = nullptr);
    void initCmds();
    void getBeamTableData(int beamTableID);
    BeamTypeRange getBeamTypeRange(QString beamtype);
    int findClosestBeamID(double targetAz, double targetEl,
                          QString beamtype="NARROW", int beamfactor=1) override;
    int getBestBeamID(double minaz, double maxaz, double minel, double maxel) override;
    QList<int> getIntList(QString key) override;
    double getTargetEIRP(double dist);
    QVector<double> getTxAtt(double distance) override;
    QVector<double> getRxAtt(double distance) override;
    QVector<double> getBFAtt(double distance) override;
    QVector<double> getBFRxAtt(double distance);
    QVector<double> getBFTxAtt(double distance);
    QString getCmd(QString key) override;

    QString getFreqIdx(double ghz);
    QVector<QVector<double>> getBeamTableDatas(int limitid=240);
    QVector<QVector<double>> getBeamTableDatas(QString beamtype);
    QVector<int> findNearestNeighbors(int targetID, QString beamtype="NARROW",
                                      int neighborGroup = 1);

public slots:
    void initBeamData(QString filename) override;
    void initBeamData(QIODevice *device);

signals:
    void updateRefFile(QString filename);
    void newBeamTableIDs(QStringList keys);
    void updateBeamTableData(double az, double el, double azBW, double elBW);
    void updateBeamTypes(QStringList beamtypes);
    void updateBeamTypeGroup(QMap<QString, QList<int>> data);
private:
    QMap<int, HanwhaBeamTableData> *mBeamTableData;
    QMap<QString, QList<int>> mBeamTypeData; // store beamtype "NERROW"... beamDirection ID list
    QMap<QString, BeamTypeRange> mBeamTypeRangeData; // store beamtype "NERROW"... min/max AZ/EL
    QJsonObject cmdObj;
    QJsonObject mRangeDataObj;
};

#endif // HANWHA_H
