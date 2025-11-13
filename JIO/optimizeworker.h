#ifndef OPTIMIZEWORKER_H
#define OPTIMIZEWORKER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

#include "perfmetricsstore.h"

class OptimizeWorker : public QObject
{
    Q_OBJECT
public:
    explicit OptimizeWorker(QJsonObject initdata, QObject *parent = nullptr);
    void setStop(bool stop);
public slots:
    void work();
    void Stop();
    void log(QString msg, int lv=3);
    void addIperf(QString server, QString client, int duration, int port=5201);
signals:
    void started();
    void stoped(int error);
    void debugMsg(QString msg);
    void sigAddIperf(QString cfg);
    void sigStartIperf(QString cfg);
    void sigClearIperf();
    void sigAddData(QDateTime testtime, int apbeamid,
                 QString clientname, int clientbeamid);
private:
    int mDebugLv;
    QDateTime currentDateTime;
    QJsonObject mInitData;
    QJsonArray mPosArr;
    int mDuration;
    bool mStop;
    QString mLocalAddr;
    QString mAPAddr;
    QString mAPMac;
    int mAIP1BeamID;
    int mAIP2BeamID;
    // QJsonObject mRecord;
    PerfMetricsStore mRecord; // store test record

};

#endif // OPTIMIZEWORKER_H
