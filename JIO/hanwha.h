#ifndef HANWHA_H
#define HANWHA_H

#include <QObject>
#include "aip.h"
#include "hanwhabeamtabledata.h"

class Hanwha : public AIP
{
    Q_OBJECT
public:
    explicit Hanwha(QObject *parent = nullptr);
    void getBeamTableData(int beamTableID);
    QVector<QVector<double>> getBeamTableDatas(int limitid=240);
    QVector<QVector<double>> getBeamTableDatas(QString beamtype);
public slots:
    void initBeamData(QString filename);
    void initBeamData(QIODevice *device);

signals:
    void updateRefFile(QString filename);
    void newBeamTableIDs(QStringList keys);
    void updateBeamTableData(double az, double el, double azBW, double elBW);
    void updateBeamTypes(QStringList beamtypes);
    void updateBeamTypeGroup(QMap<QString, QStringList> data);
private:
    QMap<int, HanwhaBeamTableData> *mBeamTableData;
    QMap<QString, QStringList> mBeamTypeData;
};

#endif // HANWHA_H
